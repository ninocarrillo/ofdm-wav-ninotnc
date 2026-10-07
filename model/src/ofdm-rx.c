#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ofdm-rx.h"
#include "textfile.h"
#include "ofdm.h"
#include "wavfile.h"
#include "dsp.h"
#include "ofdm-dev.h"
#include "edac.h"

int main(int arg_count, char* arg_values[]) {
	
	if (arg_count < 3) {
		printf("Not enough arguments.\r\n");
		printf("Usage:\r\nofdm-rx.exe <input wav file> <output text file>\r\n");
		return(-1);
	}
	
	// Attempte to open input wav file
	FILE *input_wav_file;
	input_wav_file = fopen(arg_values[1], "rb");
	if (input_wav_file == NULL) {
		printf("Could not open %s.\r\n", arg_values[1]);
		return(-1);
	}

	// Structure for wav file header read from input file
	WAVHdr_struct wav_header;

	// Attempt to read wav file header
	fread(&wav_header, sizeof(wav_header), 1, input_wav_file);

	// Check for correct sample rate.
	if (WavHdrGetSampleRate(&wav_header) != OFDM_SYS_SAMPLE_RATE) {
		printf("Sample Rate of %s is %i, should be %i.\r\n", arg_values[1], WavHdrGetSampleRate(&wav_header), OFDM_SYS_SAMPLE_RATE);
		return(-1);
	}
	// Check for correct channel count.
	if (WavHdrGetChannelCount(&wav_header) != 1) {
		printf("Channel count of %s is %i, should be %i.\r\n", arg_values[1], WavHdrGetChannelCount(&wav_header), 1);
		return(-1);
	}

	// Get number of samples in input wav file.
	int sample_count = WavHdrGetSampleCount(&wav_header);
	printf("%s contains %i samples.\r\n", arg_values[1], sample_count);

	FILE *output_file;
	output_file = fopen(arg_values[2], "wb");
	if (output_file == NULL) {
		printf("Could not create %s.\r\n", arg_values[2]);
		return(-1);
	}

	OFDM_Rx_struct Receiver;
	OFDMRxInit(&Receiver);

	OFDM_Dev_struct OFDM_Dev;

	uint8_t output_data[MAX_PAYLOAD+2];
	int packet_payload_count;

	int16_t input_buffer[2048];
	int read_count = 0;
	long int absolute_offset = 0;
	int sync_count = 0;
	int invalid_sync_count = 0;
	int packet_count = 0;
	int reject_count = 0;
	int symbol_count = 0;
	int16_t dc_offset = 0;
	while((read_count = fread(input_buffer, sizeof(input_buffer[0]), 2048, input_wav_file)) > 0) {
		for (int i = 0; i < read_count; i++) {
				int16_t sync_detect = 0;
				int decoded_byte_count = 0;
				switch(Receiver.State) {
				case OFDM_RX_SEARCH:
					sync_detect = OFDMSyncSearch(&Receiver, input_buffer[i]);
					OFDMSyncDump(&Receiver, &OFDM_Dev);
					if (sync_detect > 0) {
						sync_count++;
						// Sync is detected. The Schmidl-Cox preamble symbol is in the receiver circ buf.
						// Process the sync symbol to generate the Equalizer taps.
						OFDMProcessSyncSymbol(&Receiver);
						
						// Draw some SVG files
						OFDMEqSVG(&Receiver, sync_count);
						OFDMEqMagSVG(&Receiver, sync_count);
						OFDMSyncSVG(&Receiver, &OFDM_Dev, sync_count);
						
						if (Receiver.ValidSyncCRC > 0) {
							printf("Valid Sync detected. Delay: %i, Indicated offset: %li\r\n", Receiver.Delay, absolute_offset-(long int)Receiver.Delay);
							printf("**** Constellation: %i\r\n", Receiver.Syncfield.Fields.Constellation);
							printf("**** Randomizer %i\r\n", Receiver.Syncfield.Fields.RandomizerIndex);
							printf("**** Payload Byte Count: %i\r\n", Receiver.Syncfield.Fields.PayloadByteCount);
							printf("**** Block Code: %i\r\n", Receiver.Syncfield.Fields.BlockCode);
							printf("Signal E %i\r\n", Receiver.SignalE);
							printf("Noise E %i\r\n", Receiver.NoiseE);
							printf("Est SNR: %i\r\n", CalcDecibelEnergy(Receiver.SignalE, Receiver.NoiseE));
						} else {
							invalid_sync_count++;
							printf("******* Invalid Sync detected. Delay: %i, Indicated offset: %li\r\n", Receiver.Delay, absolute_offset-(long int)Receiver.Delay);
						}
						if (Receiver.Syncfield.Fields.PayloadByteCount > 0) {
							// Only change to receive state if there are data bytes to follow.
							Receiver.DemodWordBitCount = 0;
							Receiver.State = OFDM_RX_DATA;
							Receiver.Randomizer = OFDM_RAND_SEED;
							Receiver.Syncfield.Fields.Bandwidth = 0;
							packet_payload_count = 0;
							symbol_count = 0;
						} else {
							fprintf(output_file, "\r\n");
						}
					}
					break;
				case OFDM_RX_DATA:
					decoded_byte_count = OFDMDataReceive(&Receiver, input_buffer[i]);
					if (decoded_byte_count > 0) {
						for (int i = 0; i < decoded_byte_count; i++) {
							output_data[packet_payload_count++] = Receiver.DataBuf[i];
						}
						OFDMSymbolSVG(&Receiver, sync_count, symbol_count++);
					}
					if (Receiver.BytesRemaining <= 0) {
						// Calculate CRC for payload and put it at the end
						if (packet_payload_count > 0) {
							if (Receiver.Syncfield.Fields.BlockCode) {
								packet_payload_count = EDACDecodeRS2(output_data, packet_payload_count, OFDM_BLOCK_CODE_MAX, OFDM_BLOCK_CODE_PARITY);
							}
							packet_payload_count = EDACCheckCCITT16(output_data, packet_payload_count);
							if (packet_payload_count > 0) {
								packet_count++;
								printf("Good Packet CRC.\r\n");
								for (int i = 0; i < packet_payload_count; i++) {
									printf("%c", output_data[i]);
								}
								printf("\r\n");
								fwrite(output_data, packet_payload_count, 1, output_file);
								fprintf(output_file, "\r\n");
							} else {
								reject_count++;
								printf("Bad Packet CRC.\r\n");
							}
						}
						Receiver.State = OFDM_RX_SEARCH;
						//OFDMClearBuffers(&Receiver);
					}
					break;
				}
			
			absolute_offset++;
		}
	}
	
	fclose(input_wav_file);
	fclose(output_file);

	printf("Valid sync symbols detected: %i\r\n", sync_count);
	printf("Invalid sync symbols detected: %i\r\n", invalid_sync_count);
	printf("Valid packets decoded: %i\r\n", packet_count);
	printf("Packets rejected for CRC: %i\r\n", reject_count);
	printf("Done.\r\n");

	return(packet_count);
}