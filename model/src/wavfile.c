#include "wavfile.h"


void InitWavHdr(WAVHdr_struct *header) {
	header->FileTypeBlockID[0] = 'R';
	header->FileTypeBlockID[1] = 'I';
	header->FileTypeBlockID[2] = 'F';
	header->FileTypeBlockID[3] = 'F';
	header->FileSize = 36;
	header->FileFormatID[0] = 'W';
	header->FileFormatID[1] = 'A';
	header->FileFormatID[2] = 'V';
	header->FileFormatID[3] = 'E';
	header->FormatBlockID[0] = 'f';
	header->FormatBlockID[1] = 'm';
	header->FormatBlockID[2] = 't';
	header->FormatBlockID[3] = ' ';
	header->BlockSize = 16;
	header->AudioFormat = 1;
	header->ChannelCount = 1;
	header->SampleRate = 24000;
	header->BytesPerSec = 48000;
	header->BytesPerBlock = 2;
	header->BitsPerSample = 16;
	header->DataBlockID[0] = 'd';
	header->DataBlockID[1] = 'a';
	header->DataBlockID[2] = 't';
	header->DataBlockID[3] = 'a';
	header->DataSize = 0;
};

void WavHdrAddSamples(WAVHdr_struct *header, int count) {
	int add_size = (count * header->ChannelCount * header->BitsPerSample) / 8;
	header->FileSize += add_size;
	header->DataSize += add_size;
	//printf("Added %i samples\r\n", add_size);
}

void WavHdrSetSampleRate(WAVHdr_struct *header, int sample_rate) {
	header->SampleRate = sample_rate;
	header->BytesPerSec = header->SampleRate * header->BitsPerSample * header->ChannelCount / 8;
}

int WavHdrGetSampleRate(WAVHdr_struct *header) {
	return header->SampleRate;
}

int WavHdrGetChannelCount(WAVHdr_struct *header) {
	return header->ChannelCount;
}

int WavHdrGetSampleCount(WAVHdr_struct *header) {
	return (header->DataSize * 8) / (header->ChannelCount * header->BitsPerSample);
}

void WavHdrPrint(WAVHdr_struct *header) {
	printf("wav header dump:\r\n");
	printf("File Type Block ID: ");
	for (int i = 0; i < 4; i++) {
			printf("%c", header->FileTypeBlockID[i]);
	}
	printf("\r\n");
	printf("File Size: %i", header->FileSize);
	printf("\r\n");
	printf("File Format ID: ");
	for (int i = 0; i < 4; i++) {
			printf("%c", header->FileFormatID[i]);
	}
	printf("\r\n");
	printf("Format Block ID: ");
	for (int i = 0; i < 4; i++) {
			printf("%c", header->FormatBlockID[i]);
	}
	printf("\r\n");
	printf("Block Size: %i", header->BlockSize);
	printf("\r\n");
	printf("Audio Format: %i", header->AudioFormat);
	printf("\r\n");
	printf("Channel Count: %i", header->ChannelCount);
	printf("\r\n");
	printf("Sample Rate: %i", header->SampleRate);
	printf("\r\n");
	printf("Bytes Per Sec: %i", header->BytesPerSec);
	printf("\r\n");
	printf("Bytes Per Block: %i", header->BytesPerBlock);
	printf("\r\n");
	printf("Bits Per Sample: %i", header->BitsPerSample);
	printf("\r\n");
	printf("Data Block ID: ");
	for (int i = 0; i < 4; i++) {
			printf("%c", header->DataBlockID[i]);
	}
	printf("\r\n");
	printf("Data Size: %i", header->DataSize);
	printf("\r\n");
}