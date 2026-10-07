#ifndef WAVFILE_H
#define WAVFILE_H
#include <stdint.h>
#include <stdio.h>

typedef struct {
	char FileTypeBlockID[4];
	uint32_t FileSize;
	char FileFormatID[4];
	char FormatBlockID[4];
	uint32_t BlockSize;
	uint16_t AudioFormat;
	uint16_t ChannelCount;
	uint32_t SampleRate;
	uint32_t BytesPerSec;
	uint16_t BytesPerBlock;
	uint16_t BitsPerSample;
	char DataBlockID[4];
	uint32_t DataSize;
} WAVHdr_struct;

void InitWavHdr(WAVHdr_struct *);
void WavHdrAddSamples(WAVHdr_struct *, int);
void WavHdrSetSampleRate(WAVHdr_struct *, int);
int WavHdrGetSampleRate(WAVHdr_struct*);
int WavHdrGetChannelCount(WAVHdr_struct*);
int WavHdrGetSampleCount(WAVHdr_struct *);
void WavHdrPrint(WAVHdr_struct *);

#endif