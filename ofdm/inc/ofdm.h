/* 
 * File:   ofdm.h
 * Author: ninoc
 *
 * Created on April 13, 2026, 7:24 PM
 */

#ifndef OFDM_H
#define	OFDM_H
#include <stdint.h>
#define OFDM_CP_N 16
#define OFDM_FFT_N 256
#define OFDM_FFT_STAGE_N 8
#define OFDM_SC_REG_BITS 11
#define OFDM_FFT_SAMPLE_RATE 6000
#define OFDM_BIN_HZ ((double)OFDM_FFT_SAMPLE_RATE / (double)OFDM_FFT_N)
#define OFDM_PREAMBLE_SAMPLE_COUNT 480
#define OFDM_PREAMBLE_BIN_1 47
#define OFDM_PREAMBLE_BIN_2 83
#define OFDM_PREAMBLE_TONE_1 (OFDM_BIN_HZ * OFDM_PREAMBLE_BIN_1)
#define OFDM_PREAMBLE_TONE_2 (OFDM_BIN_HZ * OFDM_PREAMBLE_BIN_2)
#define OFDM_AGC_FREEZE_HOLD ((OFDM_FFT_N + OFDM_CP_N) * 4)

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

#define OFDM_CONST_MASK 0x7


#define OFDM_BUF2_N ((OFDM_CP_N<<1) + 1)
#define OFDM_BUF_N ((OFDM_FFT_N + OFDM_BUF2_N) + ((OFDM_FFT_N + OFDM_BUF2_N)>>1))

#define OFDM_PILOT_I 32767
#define OFDM_PILOT_Q 0

#define OFDM_PILOT_N 3
#define OFDM_START_BIN 18
#define OFDM_END_BIN 114
#define OFDM_SYNCFIELD_BIN(BIT_INDEX) (OFDM_START_BIN + 2 + (BIT_INDEX<<2))

// Scaling factor applied during equalization to allow for
// positive gains with integer math. Lower number means
// more bits are left of the decimal place and equalizer
// can therefore provide more amplification.
#define OFDM_CONST_SHIFT 10

// Scaling metric applied to time-domain samples before RFFT to prevent
// saturation. Higher number provides more attenuation but reduces 
// dynamic range.
// Number represents right bit shift.
#define OFDM_FFT_SHIFT 3

// Scaling metric applied to time-domain samples after RIFFT.
// Number represents left bit shift.
#define OFDM_IFFT_SHIFT 2

#define OFDM_PRESYNC_SHIFT 2

#define OFDM_RAND_SEED 0xED

#define OFDM_MAX_SYMBOL_DATA (OFDM_FFT_N>>1) // Maximum number of bytes carried in a single symbol

#define OFDM_SYNCWORD_DATA_N 24
#define OFDM_SYNCWORD_CRC_N 8
#define OFDM_SYNCFIELD_MASK (((1<<(OFDM_SYNCWORD_DATA_N-OFDM_SYNCWORD_CRC_N))-1)<<OFDM_SYNCWORD_CRC_N)

#define OFDM_SC_P1_CUTOFF 25

#define OFDM_PRESYNC_MIN_SNR 7

#define OFDM_SC_ARM_E_THRESH 500

// OFDM_BLOCK_CODE_MAX + OFDM_BLOCK_CODE_PARITY <= 255
#define OFDM_BLOCK_CODE_MAX 239
#define OFDM_BLOCK_CODE_PARITY 16

#define OFDM_SYNCFIELD_RANDOMIZER_BITS 2
#define OFDM_SYNCFIELD_RANDOMIZER_MASK ((1<<OFDM_SYNCFIELD_RANDOMIZER_BITS)-1)

typedef union {
	struct {
		uint32_t CRC8             :8;
		uint32_t FEC              :1;
		uint32_t PerfTrailer      :1;
		uint32_t RandomizerIndex  :OFDM_SYNCFIELD_RANDOMIZER_BITS;
		uint32_t Constellation    :3;
		uint32_t PayloadByteCount :9;
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
    int RandomizerIndex;
	int WordBits;
	int PayloadBits;
    int TxPreambleCount;
    int ExtendedPayloadByteCount;
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
	int16_t EqLowBinMag;
	int16_t EqMidBinMag;
	int16_t EqHighBinMag;
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
    uint16_t EnableAGC :1;
} OFDM_Rx_struct;


uint16_t StepRandomizer(uint16_t *, int );
int OFDMBuildSyncSymbol(OFDM_Tx_struct *);
int OFDMBuildDataSymbol(OFDM_Tx_struct *);
void OFDMRxInit(OFDM_Rx_struct *);
int OFDMSyncSearch(OFDM_Rx_struct *, int16_t);
void OFDMProcessSyncSymbol(OFDM_Rx_struct *);
int OFDMDataReceive(OFDM_Rx_struct *, int16_t);
int OFDMDecode(OFDM_Rx_struct *);

#ifdef	__cplusplus
}
#endif

#endif	/* OFDM_H */

