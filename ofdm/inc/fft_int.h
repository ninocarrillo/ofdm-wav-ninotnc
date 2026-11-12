#ifndef fft_int_h
#define fft_int_h
#include <stdint.h>
#define FFT_DESIGN_STAGE_COUNT 10
void RFFT(int16_t *, int);
void RIFFT(int16_t *, int);
void ComplexRotate_q15(int16_t *, int);
#endif