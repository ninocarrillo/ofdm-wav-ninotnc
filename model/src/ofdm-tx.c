#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "ofdm-tx.h"
#include "textfile.h"
#include "ofdm.h"
#include "wavfile.h"
#include "dsp.h"
#include "edac.h"

int16_t calc_avg_amplitude(int16_t *buf, int count) {
	int64_t sum = 0;
	for (int i = 0; i < count; i++) {
		if (buf[i] >= 0) {
			sum += (int64_t)buf[i];
		} else {
			sum -= (int64_t)buf[i];
		}
	}
	return sum / (int64_t)count;
}

int16_t calc_peak_amplitude(int16_t *buf, int count) {
	int16_t peak = 0;
	for (int i = 0; i < count; i++) {
		if (buf[i] >= 0) {
			if (buf[i] > peak) {
				peak = buf[i];
			}
		} else {
			if ((-buf[i]) > peak) {
				peak = -buf[i];
			}
		}
	}
	return peak;
}

int16_t calc_rms_amplitude(int16_t *buf, int count) {
	int64_t sum = 0;
	for (int i = 0; i < count; i++) {
		sum += (buf[i] * buf[i]);
	}
	sum = sum / (int64_t) count;
	return sqrt(sum);
}

void clip_samples(int16_t *buf, int count, int max) {
	for (int i = 0; i < count; i++) {
		if (buf[i] > 0) {
			if (buf[i] > max) {
				buf[i] = max;
			}
		} else {
			if (buf[i] < -max) {
				buf[i] = -max;
			}
		}
	}
}

int main(int arg_count, char* arg_values[]) {
	
	if (arg_count < 7) {
		printf("Not enough arguments.\r\n");
		printf("Usage:\r\nofdm-tx.exe <input text file> <output wav file> <constellation> <FEC> <presync 20ms> <interp rate>\r\n");
		return(-1);
	}

	int const_arg = atoi(arg_values[3]);

	int block_code_arg = atoi(arg_values[4]);

	int presync_arg = atoi(arg_values[5]);

	int interp_rate_arg = atoi(arg_values[6]);
	
	FILE *input_file;
	input_file = fopen(arg_values[1], "r");
	if (input_file == NULL) {
		printf("Could not open %s.\r\n", arg_values[1]);
		return(-1);
	}
	
	FILE *output_wav_file;
	output_wav_file = fopen(arg_values[2], "wb");
	if (output_wav_file == NULL) {
		printf("Could not create %s.\r\n", arg_values[2]);
		return(-1);
	}

	if ((interp_rate_arg > OFDM_TX_MAX_INTERP_RATE) || (interp_rate_arg < 1)) {
		printf("Interp rate must be between 1 and 8.\r\n");
		return(-1);
	}

	Interpolator_struct Interpolator;
	Interpolator.Rate = interp_rate_arg;
	Interpolator.TapN = OFDM_TX_INTERP_FIR_N;
	Interpolator.CircN = DSP_MAX_FIR_TAPS;
	Interpolator.CircIndex = 0;
	GenInterpFIR2(Interpolator.Taps, OFDM_TX_INTERP_FIR_CUTOFF, OFDM_FFT_SAMPLE_RATE * interp_rate_arg, interp_rate_arg, Interpolator.TapN);
	
	
	WAVHdr_struct wav_header;
	InitWavHdr(&wav_header);
	WavHdrSetSampleRate(&wav_header, OFDM_FFT_SAMPLE_RATE * interp_rate_arg);
	
	// move the output file offset past the header
	fseek(output_wav_file, sizeof(wav_header), SEEK_SET);
	
	int packet_count = txtCountLines(input_file);
	
	printf("%s contains %i lines.\r\n", arg_values[1], packet_count);

	OFDM_Tx_struct Transmitter;
	Transmitter.Syncfield.Fields.RandomizerIndex = 0;
	Transmitter.Randomizer = OFDM_RAND_SEED;

	// Buffers for the baseband samples and the interpolated samples. Buffer size is one symbol plus cyclic prefix
	int16_t baseband_samples[OFDM_FFT_N + OFDM_CP_N + DSP_MAX_FIR_TAPS];
	int16_t interpolated_samples[OFDM_TX_MAX_INTERP_RATE * (OFDM_FFT_N + OFDM_CP_N + DSP_MAX_FIR_TAPS)];

	// Test Randomizer
	printf("Randomizer Test: %i\r\n", Transmitter.Syncfield.Fields.RandomizerIndex);
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			printf("%02x ", StepRandomizer(&Transmitter.Randomizer, Transmitter.Syncfield.Fields.RandomizerIndex));
		}
		printf("\r\n");
	}
	
	// Treat each line of the text file as a packet.
	// Generate an OFDM transmission for each packet.
	int zero_length_count = 0;
	for (int i = 0; i < packet_count; i++) {
		

		// A buffer for the payload data
		uint8_t input_data[OFDM_TX_BUF_LEN];
		
		// Read one line of the input text file
		fgets((char *)input_data, OFDM_TX_BUF_LEN, input_file);
		

		// Generate sync symbol
		Transmitter.RandomizerIndex = (Transmitter.RandomizerIndex++) & OFDM_SYNCFIELD_RANDOMIZER_MASK;
		Transmitter.BasebandSamples = baseband_samples; // Put pointer to baseband samples in the Transmitter struct
		Transmitter.Randomizer = OFDM_RAND_SEED;
		Transmitter.Syncfield.Word = 0;
		Transmitter.Syncfield.Fields.Constellation = const_arg & 7;
		Transmitter.Syncfield.Fields.FEC = block_code_arg & 1;
		Transmitter.Syncfield.Fields.RandomizerIndex = Transmitter.RandomizerIndex;
		Transmitter.TxPreambleCount = presync_arg;

		// Count how many characters this line contains
		int packet_payload_count = txtCountChars(input_data);
		Transmitter.Syncfield.Fields.PayloadByteCount = packet_payload_count;
		
		// Only add a packet CRC if the payload count is nonzero. Zero payload packets are only a sync symbol.
		if (packet_payload_count > 0) {
			// Calculate CRC for payload and put it at the end
            packet_payload_count = EDACAppendCCITT16(input_data, packet_payload_count);
            printf("Packet Payload Count: %i, Packet CRC: %04x\r\n", packet_payload_count, EDACGetCCITT16(input_data, packet_count));
            if (Transmitter.Syncfield.Fields.FEC) {
            	packet_payload_count = EDACEncodeRS2(input_data, packet_payload_count, OFDM_BLOCK_CODE_MAX, OFDM_BLOCK_CODE_PARITY);
            	printf("Packet Payload Count: %i after RS encode.\r\n", packet_payload_count);
        	}
		} else {
			zero_length_count++;
			printf("Zero-length packet, sync symbol only.\r\n");
		}
	

		for (int j = 0; j < presync_arg; j++) {
			// Put some silence before each transmission.
			int silence_count = OFDM_PREAMBLE_SAMPLE_COUNT;
			for (int i = 0; i < silence_count; i++) {
				baseband_samples[i] = 0;
			}
			int interpolated_sample_count = Interpolate2(&Interpolator, baseband_samples, silence_count, interpolated_samples, -Interpolator.Rate/2);
			fwrite(interpolated_samples, sizeof(interpolated_samples[0]), interpolated_sample_count, output_wav_file);
			// Update wavfile header with new sample count
			WavHdrAddSamples(&wav_header, interpolated_sample_count);
		}


		while (Transmitter.TxPreambleCount > 0) {
			int baseband_sample_count = OFDMBuildSyncSymbol(&Transmitter);

			int16_t avg = calc_avg_amplitude(baseband_samples, baseband_sample_count);
			int16_t peak = calc_peak_amplitude(baseband_samples, baseband_sample_count);
			int16_t rms = calc_rms_amplitude(baseband_samples, baseband_sample_count);
			printf("Pre Sym Peak sample amplitude: %i\r\n", peak);
			printf("Pre Sym RMS sample amplitude: %i\r\n", rms);
			printf("Pre Sym Avg sample amplitude: %i\r\n", avg);
			printf("Pre Sym Peak to RMS dB: %0.1f\r\n", 20*(log10((float)peak/(float)rms)));
			printf("Pre Sym Peak to Avg dB: %0.1f\r\n", 20*(log10((float)peak/(float)avg)));

			
			// Write symbol to the output file
			int interpolated_sample_count = Interpolate2(&Interpolator, baseband_samples, baseband_sample_count, interpolated_samples, -Interpolator.Rate/2);
			size_t written = fwrite(interpolated_samples, sizeof(interpolated_samples[0]), interpolated_sample_count, output_wav_file);
			// Update wavfile header with new sample count
			WavHdrAddSamples(&wav_header, interpolated_sample_count);
			printf("Baseband contained %i samples of %lu bytes each.\r\n", baseband_sample_count, sizeof(baseband_samples[0]));
			printf("Interpolated %i samples of %lu bytes each.\r\n", interpolated_sample_count, sizeof(interpolated_samples[0]));
			printf("Interpolated sample buffer is %lu bytes long.\r\n", sizeof(interpolated_samples));
			printf("Fwrite wrote %zu elements.\r\n", written);
		}

		Transmitter.ExtendedPayloadByteCount = packet_payload_count;
		// Add the data symbols after this sync symbol. Zero-length payloads contain zero data symbols.
		Transmitter.PayloadByteIndex = 0;
		Transmitter.WorkingWord = 0;
		Transmitter.WordBits = 0;
		Transmitter.PayloadBits = Transmitter.ExtendedPayloadByteCount * 8;
		Transmitter.BasebandSamples = baseband_samples;
		Transmitter.InputData = input_data;
		while (Transmitter.PayloadBits > 0) {
			printf("Building Data Symbol. Index: %i, Count: %i, Bits Remaining: %i\r\n", Transmitter.PayloadByteIndex, packet_payload_count, Transmitter.PayloadBits);

			int baseband_sample_count = OFDMBuildDataSymbol(&Transmitter);


			int16_t avg = calc_avg_amplitude(baseband_samples, baseband_sample_count);
			int16_t peak = calc_peak_amplitude(baseband_samples, baseband_sample_count);
			int16_t rms = calc_rms_amplitude(baseband_samples, baseband_sample_count);
			printf("Data Sym Peak sample amplitude: %i\r\n", peak);
			printf("Data Sym RMS sample amplitude: %i\r\n", rms);
			printf("Data Sym Avg sample amplitude: %i\r\n", avg);
			printf("Data Sym Peak to RMS dB: %0.1f\r\n", 20*(log10((float)peak/(float)rms)));
			printf("Data Sym Peak to Avg dB: %0.1f\r\n", 20*(log10((float)peak/(float)avg)));
			
			
			int interpolated_sample_count = Interpolate2(&Interpolator, baseband_samples, baseband_sample_count, interpolated_samples, -Interpolator.Rate/2);
			size_t written = fwrite(interpolated_samples, sizeof(interpolated_samples[0]), interpolated_sample_count, output_wav_file);
			// Update wavfile header with new sample count
			WavHdrAddSamples(&wav_header, interpolated_sample_count);
			printf("Baseband contained %i samples of %lu bytes each.\r\n", baseband_sample_count, sizeof(baseband_samples[0]));
			printf("Preamble interpolated %i samples of %lu bytes each.\r\n", interpolated_sample_count, sizeof(interpolated_samples[0]));
			printf("Interpolated sample buffer is %lu bytes long.\r\n", sizeof(interpolated_samples));
			printf("Fwrite wrote %zu elements.\r\n", written);
		}
	}
	

	int silence_count = DSP_MAX_FIR_TAPS;

	// Put some silence at the end of the file.
	for (int i = 0; i < silence_count; i++) {
		baseband_samples[i] = 0;
	}
	int interpolated_sample_count = Interpolate2(&Interpolator, baseband_samples, silence_count, interpolated_samples, -Interpolator.Rate/2);
	fwrite(interpolated_samples, sizeof(interpolated_samples[0]), interpolated_sample_count, output_wav_file);
	// Update wavfile header with new sample count
	WavHdrAddSamples(&wav_header, interpolated_sample_count);
	
	// Seek to the 0 position of the output wav file to put the header there
	fseek(output_wav_file, 0, SEEK_SET);

	// Write the wav header to the file
	fwrite(&wav_header, sizeof(wav_header), 1, output_wav_file);
	printf("Wrote %i samples to %s.\r\n", WavHdrGetSampleCount(&wav_header), arg_values[2]);
	WavHdrPrint(&wav_header);

	fclose(output_wav_file);
	fclose(input_file);

	printf("Total packets generated: %i\r\n", packet_count);
	printf("Zero-length packet count: %i\r\n", zero_length_count);
	printf("Data packet count: %i\r\n", packet_count-zero_length_count);
	printf("OFDM SYNCFIELD MASK %x\r\n", OFDM_SYNCFIELD_MASK);
	printf("Done.\r\n");
	return(0);
}