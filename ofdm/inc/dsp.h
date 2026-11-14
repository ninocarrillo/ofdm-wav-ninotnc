/* 
 * File:   dsp.h
 * Author: ninoc
 *
 * Created on September 5, 2025, 1:43 PM
 */

#ifndef DSP_H
#define	DSP_H

#define DSP_MAX_FIR_TAPS 501
#define DSP_NORM_FREQ 86400

#include <stdint.h>

typedef struct {
	int16_t CircBuf[DSP_MAX_FIR_TAPS];
	int16_t Taps[DSP_MAX_FIR_TAPS];
	int TapN;
	int CircIndex;
	int CircN;
	int Rate;
} Interpolator_struct;

typedef struct {
    int16_t LastValue;
    int16_t Envelope;
    int16_t AttackRate;
    int16_t DecayRate;
    int16_t SustainPeriod;
    int16_t SustainCount;
} Int16_Peak_Detector_struct;

typedef struct {
    int16_t a1;
    int16_t b0;
    int16_t b1;
    int16_t x0;
    int16_t x1;
    int16_t y1;
} IIR_Order1_struct;

typedef struct {
	Int16_Peak_Detector_struct PeakDetector;
	IIR_Order1_struct HPF;
	int16_t CircBuf[DSP_MAX_FIR_TAPS];
	int16_t Taps[DSP_MAX_FIR_TAPS];
	int16_t Output;
	int16_t PrescalerShift;
	int TapN;
	int CircIndex;
	int CircN;
	int DeciIndex;
	int Rate;
	uint16_t EnableAGC :1;
} Decimator_struct;

extern const int16_t  SquareRootTable[];

void GenLPFIR2(int16_t *, int32_t, int32_t, int16_t, int16_t);
void GenHPFIR3(int16_t *, int32_t, int32_t, int16_t, int16_t);
void GenBandFIR3(int16_t *, int32_t, int32_t, int32_t, int16_t, int16_t);
void GenCorrelator2(int16_t *, int32_t, int32_t, int16_t, int16_t, int16_t, int16_t);
int16_t GenHilbertFIR(int16_t *, int16_t );
void GenInterpFIR(int16_t *, int, int);
void GenInterpFIR2(int16_t *, int32_t, int32_t, int, int);
void GenInterpFIR3(int16_t *, int32_t, int32_t, int32_t, int, int);
void GenInterpFIR4(int16_t *, int32_t , int32_t , int , int , int32_t );


int16_t GetSinSample(int32_t);
int16_t GetCosSample(int32_t);
int32_t CalcPhaseAdvance(int32_t, int32_t);
int16_t HannExp(int16_t , int16_t , int16_t );
int16_t Hamming(int16_t, int16_t);
int16_t Sinc(int32_t);
int16_t GenNonNormLowPassFIR(int16_t *, int16_t, int16_t);
int16_t GetLPFTap(int32_t, int16_t, int16_t);
int16_t GetHPFTap(int32_t, int16_t, int16_t);


int16_t GetFastFilterOutput(int16_t *, int, int, int16_t *, int, int);
int Interpolate(Interpolator_struct *, int16_t *, int, int16_t *, int);
int Interpolate2(Interpolator_struct *, int16_t *, int, int16_t *, int);
int Decimate(Decimator_struct *, int16_t);

int32_t Sqrt(int32_t x);

int16_t ApproxATan2_q15(int16_t, int16_t);
int16_t ApproxATan_q15(int16_t, int16_t);
void ComplexMul_q15(int16_t*, int16_t*, int16_t*);
void ComplexMul_q13(int16_t*, int16_t*, int16_t*);
void ComplexMul_var(int16_t*, int16_t*, int16_t*, int);
int16_t EuclidianDistance_q15(int16_t *, int16_t *);

int16_t ScaledSquareRoot_q15(uint16_t);
int16_t ScaledMag_q15(int16_t*);

void GenLPMAFIR(int16_t *, int32_t, int32_t, int16_t, int16_t, int16_t);


void GenHPFIIR(IIR_Order1_struct *, int32_t , int32_t);

int16_t CalcDecibelEnergy(int32_t, int32_t);

#define DSP_PI_Q12 12867
#endif	/* DSP_H */

