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
		printf("Usage:\r\nofdm-rx <input wav file> <output text file>\r\n");
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
	int decimate = 1;
	int file_sample_rate = WavHdrGetSampleRate(&wav_header);
	int decimated_sample_rate = file_sample_rate;
	while ((decimated_sample_rate != OFDM_FFT_SAMPLE_RATE) && (decimate <= MAX_DECI_RATE)) {
		decimate++;
		decimated_sample_rate = file_sample_rate / decimate;
	}
	if (decimated_sample_rate != OFDM_FFT_SAMPLE_RATE) {
		printf("Sample Rate of %s is %i, should be multiple of %i.\r\n", arg_values[1], file_sample_rate, OFDM_FFT_SAMPLE_RATE);
		return(-1);
	}
	printf("Decimation rate is %i\r\n", decimate);
		

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

	// Generate a decimator to reduce sample rate if required.
	Decimator_struct Decimator;
	Decimator.Rate = decimate;
	Decimator.TapN = DECI_FIR_N;
	Decimator.CircIndex = 0;
	Decimator.DeciIndex = 0;
	Decimator.CircN = MAX_FIR_TAPS;
    GenLPFIR2(Decimator.Taps, DECI_FIR_CUTOFF, file_sample_rate, Decimator.TapN, 1);
    Decimator.PeakDetector.AttackRate = 5000;
    Decimator.PeakDetector.SustainPeriod = 50;
    Decimator.PeakDetector.DecayRate = 50;
    Decimator.PrescalerShift = 0;

	OFDM_Rx_struct Receiver;
	OFDMRxInit(&Receiver);

	OFDM_Dev_struct OFDM_Dev;

	uint8_t output_data[MAX_PAYLOAD+2];
	int packet_payload_count;

	int16_t input_buffer[2048];
	int read_count = 0;
	long int absolute_offset = 0;
	int sync_count = 0;
	int valid_sync_count = 0;
	int empty_packet_count = 0;
	int packet_count = 0;
	int reject_count = 0;
	int symbol_count = 0;
	int16_t dc_offset = 0;
	while((read_count = fread(input_buffer, sizeof(input_buffer[0]), 2048, input_wav_file)) > 0) {
		for (int i = 0; i < read_count; i++) {
			int sample_exists = 0;
			int16_t sample = 0;
			if (decimate > 1) {
				if (Receiver.EnableAGC) {
					Decimator.EnableAGC = 1;
				} else {
					Decimator.EnableAGC = 0;
				}
				sample_exists = Decimate(&Decimator, input_buffer[i]);
				sample = Decimator.Output;
				//sample = input_buffer[i];
			} else {
				sample_exists = 1;
				sample = input_buffer[i];
			}
			if (sample_exists) {
				int16_t sync_detect = 0;
				int decoded_byte_count = 0;
				Receiver.AGCFreezeSampleCount++;
				switch(Receiver.State) {
				case OFDM_RX_SEARCH:
		            if (Receiver.AGCFreezeSampleCount > OFDM_AGC_FREEZE_HOLD) {
	            		Receiver.AGCFreezeSampleCount = OFDM_AGC_FREEZE_HOLD;
	                	if (Receiver.EnableAGC == 0) {
	                		// Enable AGC
							printf("*** Enable AGC %li\r\n", absolute_offset);
							Receiver.EnableAGC = 1;
	                	}
	            	}
					sync_detect = OFDMSyncSearch(&Receiver, sample);
					if (Receiver.EnableAGC) { 
						if (sync_detect) {
							printf("&&&&&&&&&&&&&&&&&&&&&&&& THIS RIGHT HERE &&&&&&&&&&&&&&&&&&&&&&&&&\r\n");
						}
						sync_detect = 0;
						if (Receiver.PresyncDetectFlag) {
							printf("*** Presync Detect %li\r\n", absolute_offset);
							// Disable AGC
							Receiver.EnableAGC = 0;
							Receiver.AGCFreezeSampleCount = 0;
		                    Receiver.PresyncSNRFreeze = Receiver.PresyncSNR;
						}
					} else {
						if (Receiver.PresyncDetectFlag) {
							printf("*** Presync Redetect %li\r\n", absolute_offset);
							Receiver.AGCFreezeSampleCount = 0;
						}
					}
					OFDMSyncDump(&Receiver, &OFDM_Dev);
					if (sync_detect > 0) {
						sync_count++;
						printf("********** Sync event detected. ************\r\n");
						// Sync is detected. The Schmidl-Cox preamble symbol is in the receiver circ buf.
						// Process the sync symbol to generate the Equalizer taps.
						OFDMProcessSyncSymbol(&Receiver);
						
						// Draw some SVG files
						OFDMSyncSVG(&Receiver, &OFDM_Dev, sync_count);
						OFDMEqSVG(&Receiver, sync_count);
						OFDMEqMagSVG(&Receiver, sync_count);
						//printf("STO: %i\r\n", OFDMCalcSTO(&Receiver));
						
						if (Receiver.ValidSyncCRC > 0) {
							valid_sync_count++;
							printf("Valid Syncfield Decoded. Count: %i\r\n", sync_count);
							printf("Constellation: %i\r\n", Receiver.Syncfield.Fields.Constellation);
							printf("Randomizer %i\r\n", Receiver.Syncfield.Fields.RandomizerIndex);
							printf("Payload Byte Count: %i\r\n", Receiver.Syncfield.Fields.PayloadByteCount);
							printf("FEC Bit: %i\r\n", Receiver.Syncfield.Fields.FEC);
							printf("Signal E %i\r\n", Receiver.SignalE);
							printf("Noise E %i\r\n", Receiver.NoiseE);
							printf("Est SNR: %i\r\n", CalcDecibelEnergy(Receiver.SignalE, Receiver.NoiseE));
							printf("Eq Low Bin: [%i,%i]\r\n", Receiver.Eq[OFDM_START_BIN<<1], Receiver.Eq[(OFDM_START_BIN<<1)+1]);
							printf("Eq Low Mag: %i\r\n", Receiver.EqLowBinMag);
							printf("Eq Mid Mag: %i\r\n", Receiver.EqMidBinMag);
							printf("Eq High Mag: %i\r\n", Receiver.EqHighBinMag);

							if (Receiver.Syncfield.Fields.PayloadByteCount > 0) {
								// Only change to receive state if there are data bytes to follow.
								if (Receiver.Syncfield.Fields.FEC) {
									Receiver.BytesRemaining = EDACGetExtendedN(Receiver.BytesRemaining, OFDM_BLOCK_CODE_MAX, OFDM_BLOCK_CODE_PARITY);
									printf("Extended Byte Count with RS Overhead: %i\r\n", Receiver.BytesRemaining);
								}
								Receiver.BytesRemaining += 2;
								Receiver.DemodWordBitCount = 0;
								Receiver.State = OFDM_RX_DATA;
								Receiver.Randomizer = OFDM_RAND_SEED;
								packet_payload_count = 0;
								symbol_count = 0;
								printf("Change state to RX_DATA.\r\n");
								Receiver.AGCFreezeSampleCount = 0;
							} else {
								empty_packet_count++;
								printf("Zero byte payload, staying in state RX_SEARCH.\r\n");
								fprintf(output_file, "\r\n");
							}
						} else {
							printf("******* Invalid Sync detected. Count: %i, Delay: %i\r\n", sync_count, Receiver.Delay);
						}
					}
					break;
				case OFDM_RX_DATA:
					decoded_byte_count = OFDMDataReceive(&Receiver, sample);
					if (decoded_byte_count > 0) {
						Receiver.AGCFreezeSampleCount = 0;
						for (int i = 0; i < decoded_byte_count; i++) {
							output_data[packet_payload_count++] = Receiver.DataBuf[i];
						}
						OFDMSymbolSVG(&Receiver, sync_count, symbol_count++);
					}
					if (Receiver.BytesRemaining <= 0) {
						// Calculate CRC for payload and put it at the end
						if (packet_payload_count > 0) {
							if (Receiver.Syncfield.Fields.FEC) {
								int corrected_bytes;
								packet_payload_count = EDACDecodeRS2(output_data, packet_payload_count, OFDM_BLOCK_CODE_MAX, OFDM_BLOCK_CODE_PARITY, &corrected_bytes);
								if (corrected_bytes >= 0) {
									printf("RS Corrected %i bytes.\r\n", corrected_bytes);
									printf("Byte count after RS parity removed %i bytes.\r\n", packet_payload_count);
								} else {
									printf("RS Correction failed.\r\n");
								}
							}
							packet_payload_count = EDACCheckCCITT16(output_data, packet_payload_count);
							if (packet_payload_count > 0) {
								packet_count++;
								printf("Good Packet CRC.\r\n");
								for (int i = 0; i < packet_payload_count; i++) {
									printf("%c", output_data[i]);
								}
								printf("\r\n------------------------------");
								printf("\r\n");
								fwrite(output_data, packet_payload_count, 1, output_file);
								fprintf(output_file, "\r\n");
							} else {
								reject_count++;
								printf("---------- Bad Packet CRC.\r\n");
							}
						}
						Receiver.State = OFDM_RX_SEARCH;
					}
					break;
				}
				absolute_offset++;
			}
		}
	}
	
	fclose(input_wav_file);
	fclose(output_file);

	printf("SC sync events detected: %i\r\n", sync_count);
	printf("Valid Syncfields detected: %i\r\n", valid_sync_count);
	printf("Valid payload packets decoded: %i\r\n", packet_count);
	printf("Valid empty packets decoded: %i\r\n", empty_packet_count);
	printf("Packets rejected for CRC: %i\r\n", reject_count);
	printf("Done.\r\n");

	return(packet_count);
}