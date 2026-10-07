/* 
 * File:   ofdm.h
 * Author: ninoc
 *
 * Created on April 13, 2026, 7:24 PM
 */

#ifndef OFDM_H
#define	OFDM_H
#include <stdint.h>
#define OFDM_CP_N 64
#define OFDM_FFT_N 1024
#define OFDM_FFT_STAGE_N 10
#define OFDM_SC_REG_BITS 13
#define OFDM_FFT_SAMPLE_RATE 24000
#define OFDM_SYS_SAMPLE_RATE 24000
#define OFDM_BIN_HZ ((double)OFDM_FFT_SAMPLE_RATE / (double)OFDM_FFT_N)
#define OFDM_PREAMBLE_SAMPLE_COUNT 480
#define OFDM_PREAMBLE_TONE_1 (OFDM_BIN_HZ * 65)
#define OFDM_PREAMBLE_TONE_2 (OFDM_BIN_HZ * 67)

#ifdef	__cplusplus
extern "C" {
#endif

#define STO_ADJ (-(OFDM_CP_N>>1))

// receiver state
typedef enum ofdm_rx_states_e	{
	OFDM_RX_SEARCH=0,				// looking for sync
	OFDM_RX_SYNC,					// sync found
	OFDM_RX_DATA					// receiving data
} OFDM_RX_STATE;


// transmitter state
typedef enum ofdm_tx_states_e	{
	OFDM_TX_IDLE=0,				// looking for sync
	OFDM_TX_SYNC,				// sync found
	OFDM_TX_DATA				// receiving data
} OFDM_TX_STATE;


// Definition of constellations
// Bits per symbol is constellation define +1
typedef enum ofdm_const_e	{
	OFDM_CONST_BPSK=0,
	OFDM_CONST_QPSK,
	OFDM_CONST_8QAM,
	OFDM_CONST_16QAM,
	OFDM_CONST_32QAM,
	OFDM_CONST_64QAM,
	OFDM_CONST_128QAM,
	OFDM_CONST_256QAM,
	OFDM_CONST_MAX_BITS,
} OFDM_CONST;

// bandwidth defs
typedef enum ofdm_bw_e	{
	OFDM_BW_FM_NB=0,
	OFDM_BW_FM_ENB,
	OFDM_BW_FM_WB,
	OFDM_BW_FM_EWB
} OFDM_BW;

// FEC definitions
typedef enum ofdm_fec_e {
	OFDM_FEC_NONE=0,				// no FEC
	OFDM_FEC_CCITTCRC16,			// CCITT CRC 16
	OFDM_FEC_TRELLIS				// trellis coding
} OFDM_FEC;
#define OFDM_CONST_MASK 0x7

#define OFDM_SYNCWORD_DATA_N 26

#define OFDM_BUF2_N ((OFDM_CP_N<<1) + 1)
#define OFDM_BUF_N ((OFDM_FFT_N + OFDM_BUF2_N) + ((OFDM_FFT_N + OFDM_BUF2_N)>>1))

#define OFDM_PILOT_I 32767
#define OFDM_PILOT_Q 0

// Scaling factor applied during equalization to allow for
// positive gains with integer math. Lower number means
// more bits are left of the decimal place and equalizer
// can therefore provide more amplification.
#define OFDM_CONST_SHIFT 11

// Scaling metric applied to time-domain samples before RFFT to prevent
// saturation. Higher number provides more attenuation but reduces 
// dynamic range.
// Number represents right bit shift.
#define OFDM_FFT_SHIFT 5

// Scaling metric applied to time-domain samples after RIFFT.
// Number represents left bit shift.
#define OFDM_IFFT_SHIFT 3

// Bandwidth-variable amplitude attenuation applied after RIFFT.
// These normalize the amplitude of all modes to BW0.
// =sqrt(bw_x_bin_count / bw_0_bin_count) * 32768
// These values are applied as Q15.
#define OFDM_BW1_IFFT_ATTEN 27800
#define OFDM_BW2_IFFT_ATTEN 22164
#define OFDM_BW3_IFFT_ATTEN 19088

#define OFDM_PRESYNC_SHIFT 2

#define OFDM_RAND_SEED 0xED

#define OFDM_MAX_SYMBOL_DATA (OFDM_FFT_N>>1) // Maximum number of bytes carried in a single symbol

#define OFDM_SYNCFIELD_MASK 0x3FFFFF00;

#define OFDM_SC_P1_CUTOFF 20

#define OFDM_BLOCK_CODE_MAX 239
#define OFDM_BLOCK_CODE_PARITY 16

typedef union {
	struct {
		uint32_t CRC8             :8;
		uint32_t TrellisCode      :1;
		uint32_t BlockCode        :1;
		uint32_t PerfTrailer      :1;
		uint32_t RandomizerIndex  :2;
		uint32_t Constellation    :3;
		uint32_t PayloadByteCount :10;
		uint32_t Bandwidth        :1;
		uint32_t Unused           :5;
	} Fields;
	uint32_t Word;
} Syncfield_union;

typedef struct {
	Syncfield_union Syncfield;
	uint32_t PayloadByteIndex;
	uint16_t Randomizer;
	int16_t *BasebandSamples;
	uint8_t *InputData;
	uint16_t WorkingWord;
	int WordBits;
	int PayloadBits;
    int TxPreambleCount;
    int32_t NCO1Phase;
    int32_t NCO2Phase;
} OFDM_Tx_struct;

typedef struct {
	Syncfield_union Syncfield;
	int16_t P1CircBuf[OFDM_FFT_N>>1];
	int16_t ECircBuf[OFDM_FFT_N>>1];
	int16_t P1Index;
	int16_t CircBuf1[OFDM_BUF_N];
	int16_t SyncLPF[OFDM_BUF2_N];
	int16_t CircBuf2[OFDM_BUF2_N];
	int16_t Eq[OFDM_FFT_N];
	int16_t FFTBuf[OFDM_FFT_N];
	uint8_t DataBuf[OFDM_MAX_SYMBOL_DATA];
	uint16_t DemodWord;
	int DemodWordBitCount;
	int16_t P1;
	int16_t E;
	int16_t P1MA;
	int16_t SyncResult;
	int16_t P1Max;
	int SyncDelay;
	int SyncArm;
	int16_t SyncInhibitTimer;
	int16_t SyncArmEnergy;
	int16_t RandomizerIndex;
	uint16_t Randomizer;
	int CircIndex1;
	int CircN1;
	int SampleCount;
	int CircIndex2;
	int CircN2;
	int SyncOffset;
	int State;
	int Delay;
	int BytesRemaining;
	int WorkingBitCount;
	int ValidSyncCRC;
	uint8_t WorkingByte;
	int FirstBin;
	int LastBin;
	int ConstShift;
    int PresyncIndex;
    int PresyncSymbolCount;
	int32_t SignalE;
	int32_t NoiseE;
	int16_t AppliedPilotAngle;
    int16_t SNR;
    int16_t PresyncSNR;
    int16_t PresyncSNRFreeze;
    int32_t AGCFreezeSampleCount;
    uint16_t PresyncDetectFlag :1;
} OFDM_Rx_struct;


uint16_t StepRandomizer(uint16_t *, int );
int OFDMBuildSyncSymbol1024(OFDM_Tx_struct *);
int OFDMBuildDataSymbol1024(OFDM_Tx_struct *);
void OFDMRxInit(OFDM_Rx_struct *);
int OFDMSyncSearch(OFDM_Rx_struct *, int16_t);
void OFDMProcessSyncSymbol(OFDM_Rx_struct *);
int OFDMDataReceive(OFDM_Rx_struct *, int16_t);
int OFDMDecode(OFDM_Rx_struct *);

#ifdef	__cplusplus
}
#endif

#endif	/* OFDM_H */

