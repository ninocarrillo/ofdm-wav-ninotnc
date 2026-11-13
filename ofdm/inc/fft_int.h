#ifndef fft_int_h
#define fft_int_h
#include <stdint.h>
#define FFT_EXTERNAL_Q_BITS 15
#define FFT_DESIGN_STAGE_N 10
#define FFT_DESIGN_N 1024
void RFFT(int16_t *, int);
void RIFFT(int16_t *, int);
void ComplexRotate_q15(int16_t *, int32_t);
#endif