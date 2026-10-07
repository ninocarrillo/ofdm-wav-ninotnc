#include "ofdm.h"
#include "fft_int.h"
#include "dsp.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const int16_t OFDM_Pilot_Bins[9] = { \
/* Pilot 0 1500.0000 Hz */	64, \
/* Pilot 1 1992.1875 Hz */	85, \
/* Pilot 2 2507.8125 Hz */	107, \
/* Pilot 3 3000.0000 Hz */  128, \
/* Pilot 4 3515.6250 Hz */  150, \
/* Pilot 5 4007.8125 Hz */  171, \
/* Pilot 6 4500.0000 Hz */  192, \
/* Pilot 7 4992.1875 Hz */  213, \
/* Pilot 8 5484.3750 Hz */  234 \
};

const int OFDM_Pilot_Bin_Inv[9] = {
    512, \
    386, \
    306, \
    256, \
    218, \
    192, \
    171, \
    154, \
    140, \
};

const int16_t OFDM_Pilot_N[4] = { 3, 5, 7, 9};

const int16_t OFDM_Start_Bins[4] = { \
/* BW 0 468.75 Hz */    20, \
/* BW 1 328.125 Hz */    14, \
/* BW 2 187.500 Hz */    8, \
/* BW 3 187.500 Hz */    8 \
};

const int16_t OFDM_End_Bins[4] = { \
/* BW 0 2906.25 Hz */   124, \
/* BW 1 3984.275 Hz */   170, \
/* BW 2 5953.125 Hz */   254, \
/* BW 3 7968.75  Hz */   340 \
};

#define OFDM_SYNCFIELD_BIN(BIT_INDEX) (OFDM_Start_Bins[0] + 2 + (BIT_INDEX<<2))

const uint16_t OFDM_Syncword[16] = { \
/* 00 */ 0x030c, \
/* 01 */ 0x8a5a, \
/* 02 */ 0xf5d5, \
/* 03 */ 0x3364, \
/* 04 */ 0x5ae1, \
/* 05 */ 0x7331, \
/* 06 */ 0x3903, \
/* 07 */ 0x36b2, \
/* 08 */ 0x9067, \
/* 09 */ 0x7623, \
/* 10 */ 0x1670, \
/* 11 */ 0xad01, \
/* 12 */ 0xe09f, \
/* 13 */ 0xaa8f, \
/* 14 */ 0x8cc7, \
/* 15 */ 0xed8e, \
};

// Primitive polynomials for data randomizer.
// Each polymial is irreducible with primitive
// element a^0 = 2
const uint16_t Randomizer_Polynomials[4] = { \
/* x^13+x^4+x^3+x+1   */ 8219, \
/* x^13+x^5+x^2+x+1   */ 8231, \
/* x^13+x^5+x^4+x^2+1 */ 8245, \
/* x^13+x^6+x^4+x+1   */ 8275  \
};

/**
  * @brief Steps the randomizer 3 times and returns a pseudorandom 8-bit value in a 16-bit register.
  * @param shift_register pointer to LFSR state
  * @param poly_index selects polynomial 0 thru 3
  * @retval pseudorandom 8-bit value
  */
uint16_t StepRandomizer(uint16_t *shift_register, int poly_index) {
    for (int i = 0; i < 3; i++) {
       uint16_t feedback = (*shift_register) & Randomizer_Polynomials[poly_index & 0x3]; // save feedback
       (*shift_register) >>= 1; // shift memory register right one bit
       if (feedback & 1) { // if the feedback bit is one
          *shift_register = (*shift_register) ^ (Randomizer_Polynomials[poly_index & 0x3]>> 1); // then XOR the tapped bits
       }
    }
   return (*shift_register>>2) & 0xFF;
}

const int16_t OFDM_BPSK_MAP[4] = { \
    /* 0 */  -32767, 0, \
    /* 1 */   32767, 0  \
};

const int16_t OFDM_QPSK_MAP[8] = { \
    /* 0 */  23170,  23170, \
    /* 1 */ -23170,  23170, \
    /* 2 */ -23170, -23170, \
    /* 3 */  23170, -23170  \
};

const int16_t OFDM_8QAM_MAP[16] = { \
   /*   0   */ 0,32767, \
   /*   1   */ -21845,10922, \
   /*   2   */ 0,10922, \
   /*   3   */ 21845,10922, \
   /*   4   */ -21845,-10922, \
   /*   5   */ 0,-10922, \
   /*   6   */ 21845,-10922, \
   /*   7   */ 0,-32767, \
};

const int16_t OFDM_16QAM_MAP[32] = { \
   /*   0   */ 7723,7723, \
   /*   1   */ 23170,7723, \
   /*   2   */ 23170,23170, \
   /*   3   */ 7723,23170, \
   /*   4   */ -23170,7723, \
   /*   5   */ -7723,7723, \
   /*   6   */ -7723,23170, \
   /*   7   */ -23170,23170, \
   /*   8   */ -7723,-7723, \
   /*   9   */ -23170,-7723, \
   /*  10   */ -23170,-23170, \
   /*  11   */ -7723,-23170, \
   /*  12   */ 23170,-7723, \
   /*  13   */ 7723,-7723, \
   /*  14   */ 7723,-23170, \
   /*  15   */ 23170,-23170, \
};

const int16_t OFDM_32QAM_MAP[64] = { \
   /*   0   */ 5619,5619, \
   /*   1   */ 16858,5619, \
   /*   2   */ 28097,5619, \
   /*   3   */ 5619,16858, \
   /*   4   */ 16858,16858, \
   /*   5   */ 28097,16858, \
   /*   6   */ 5619,28097, \
   /*   7   */ 16858,28097, \
   /*   8   */ -16858,5619, \
   /*   9   */ -5619,5619, \
   /*  10   */ -5619,16858, \
   /*  11   */ -28097,5619, \
   /*  12   */ -28097,16858, \
   /*  13   */ -16858,16858, \
   /*  14   */ -16858,28097, \
   /*  15   */ -5619,28097, \
   /*  16   */ -5619,-5619, \
   /*  17   */ -16858,-5619, \
   /*  18   */ -28097,-5619, \
   /*  19   */ -5619,-16858, \
   /*  20   */ -16858,-16858, \
   /*  21   */ -28097,-16858, \
   /*  22   */ -5619,-28097, \
   /*  23   */ -16858,-28097, \
   /*  24   */ 16858,-5619, \
   /*  25   */ 5619,-5619, \
   /*  26   */ 5619,-16858, \
   /*  27   */ 28097,-5619, \
   /*  28   */ 28097,-16858, \
   /*  29   */ 16858,-16858, \
   /*  30   */ 16858,-28097, \
   /*  31   */ 5619,-28097, \
};


const int16_t OFDM_64QAM_MAP[128] = { \
   /*   0   */ 10855,3618, \
   /*   1   */ 3618,3618, \
   /*   2   */ 25329,3618, \
   /*   3   */ 18092,3618, \
   /*   4   */ 3618,10855, \
   /*   5   */ 32566,3618, \
   /*   6   */ 18092,10855, \
   /*   7   */ 10855,10855, \
   /*   8   */ 10855,18092, \
   /*   9   */ 25329,10855, \
   /*  10   */ 25329,18092, \
   /*  11   */ 3618,18092, \
   /*  12   */ 3618,25329, \
   /*  13   */ 18092,18092, \
   /*  14   */ 18092,25329, \
   /*  15   */ 10855,25329, \
   /*  16   */ -3618,3618, \
   /*  17   */ -10855,3618, \
   /*  18   */ -18092,3618, \
   /*  19   */ -25329,3618, \
   /*  20   */ -32566,3618, \
   /*  21   */ -3618,10855, \
   /*  22   */ -10855,10855, \
   /*  23   */ -18092,10855, \
   /*  24   */ -25329,10855, \
   /*  25   */ -10855,18092, \
   /*  26   */ -3618,18092, \
   /*  27   */ -25329,18092, \
   /*  28   */ -18092,18092, \
   /*  29   */ -3618,25329, \
   /*  30   */ -10855,25329, \
   /*  31   */ -18092,25329, \
   /*  32   */ -10855,-3618, \
   /*  33   */ -3618,-3618, \
   /*  34   */ -25329,-3618, \
   /*  35   */ -18092,-3618, \
   /*  36   */ -3618,-10855, \
   /*  37   */ -32566,-3618, \
   /*  38   */ -18092,-10855, \
   /*  39   */ -10855,-10855, \
   /*  40   */ -10855,-18092, \
   /*  41   */ -25329,-10855, \
   /*  42   */ -25329,-18092, \
   /*  43   */ -3618,-18092, \
   /*  44   */ -3618,-25329, \
   /*  45   */ -18092,-18092, \
   /*  46   */ -18092,-25329, \
   /*  47   */ -10855,-25329, \
   /*  48   */ 3618,-3618, \
   /*  49   */ 10855,-3618, \
   /*  50   */ 18092,-3618, \
   /*  51   */ 25329,-3618, \
   /*  52   */ 32566,-3618, \
   /*  53   */ 3618,-10855, \
   /*  54   */ 10855,-10855, \
   /*  55   */ 18092,-10855, \
   /*  56   */ 25329,-10855, \
   /*  57   */ 10855,-18092, \
   /*  58   */ 3618,-18092, \
   /*  59   */ 25329,-18092, \
   /*  60   */ 18092,-18092, \
   /*  61   */ 3618,-25329, \
   /*  62   */ 10855,-25329, \
   /*  63   */ 18092,-25329, \
};

const int16_t OFDM_128QAM_MAP[256] = { \
   /*   0   */ 7539,2513, \
   /*   1   */ 2513,2513, \
   /*   2   */ 17591,2513, \
   /*   3   */ 12565,2513, \
   /*   4   */ 27643,2513, \
   /*   5   */ 22617,2513, \
   /*   6   */ 2513,7539, \
   /*   7   */ 32669,2513, \
   /*   8   */ 12565,7539, \
   /*   9   */ 7539,7539, \
   /*  10   */ 22617,7539, \
   /*  11   */ 17591,7539, \
   /*  12   */ 7539,12565, \
   /*  13   */ 27643,7539, \
   /*  14   */ 17591,12565, \
   /*  15   */ 2513,12565, \
   /*  16   */ 27643,12565, \
   /*  17   */ 12565,12565, \
   /*  18   */ 2513,17591, \
   /*  19   */ 22617,12565, \
   /*  20   */ 12565,17591, \
   /*  21   */ 7539,17591, \
   /*  22   */ 22617,17591, \
   /*  23   */ 17591,17591, \
   /*  24   */ 7539,22617, \
   /*  25   */ 27643,17591, \
   /*  26   */ 17591,22617, \
   /*  27   */ 2513,22617, \
   /*  28   */ 2513,27643, \
   /*  29   */ 12565,22617, \
   /*  30   */ -2513,2513, \
   /*  31   */ 22617,22617, \
   /*  32   */ -12565,2513, \
   /*  33   */ 7539,27643, \
   /*  34   */ -22617,2513, \
   /*  35   */ -7539,2513, \
   /*  36   */ -32669,2513, \
   /*  37   */ -17591,2513, \
   /*  38   */ -7539,7539, \
   /*  39   */ -27643,2513, \
   /*  40   */ -17591,7539, \
   /*  41   */ -2513,7539, \
   /*  42   */ -27643,7539, \
   /*  43   */ -12565,7539, \
   /*  44   */ -2513,12565, \
   /*  45   */ -22617,7539, \
   /*  46   */ -12565,12565, \
   /*  47   */ -7539,12565, \
   /*  48   */ -22617,12565, \
   /*  49   */ -17591,12565, \
   /*  50   */ -7539,17591, \
   /*  51   */ -27643,12565, \
   /*  52   */ -17591,17591, \
   /*  53   */ -2513,17591, \
   /*  54   */ -27643,17591, \
   /*  55   */ -12565,17591, \
   /*  56   */ -2513,22617, \
   /*  57   */ -22617,17591, \
   /*  58   */ -12565,22617, \
   /*  59   */ -7539,22617, \
   /*  60   */ -22617,22617, \
   /*  61   */ -17591,22617, \
   /*  62   */ -7539,27643, \
   /*  63   */ -2513,27643, \
   /*  64   */ -7539,-2513, \
   /*  65   */ -2513,-2513, \
   /*  66   */ -17591,-2513, \
   /*  67   */ -12565,-2513, \
   /*  68   */ -27643,-2513, \
   /*  69   */ -22617,-2513, \
   /*  70   */ -2513,-7539, \
   /*  71   */ -32669,-2513, \
   /*  72   */ -12565,-7539, \
   /*  73   */ -7539,-7539, \
   /*  74   */ -22617,-7539, \
   /*  75   */ -17591,-7539, \
   /*  76   */ -7539,-12565, \
   /*  77   */ -27643,-7539, \
   /*  78   */ -17591,-12565, \
   /*  79   */ -2513,-12565, \
   /*  80   */ -27643,-12565, \
   /*  81   */ -12565,-12565, \
   /*  82   */ -2513,-17591, \
   /*  83   */ -22617,-12565, \
   /*  84   */ -12565,-17591, \
   /*  85   */ -7539,-17591, \
   /*  86   */ -22617,-17591, \
   /*  87   */ -17591,-17591, \
   /*  88   */ -7539,-22617, \
   /*  89   */ -27643,-17591, \
   /*  90   */ -17591,-22617, \
   /*  91   */ -2513,-22617, \
   /*  92   */ -2513,-27643, \
   /*  93   */ -12565,-22617, \
   /*  94   */ 2513,-2513, \
   /*  95   */ -22617,-22617, \
   /*  96   */ 12565,-2513, \
   /*  97   */ -7539,-27643, \
   /*  98   */ 22617,-2513, \
   /*  99   */ 7539,-2513, \
   /*  100  */ 32669,-2513, \
   /*  101  */ 17591,-2513, \
   /*  102  */ 7539,-7539, \
   /*  103  */ 27643,-2513, \
   /*  104  */ 17591,-7539, \
   /*  105  */ 2513,-7539, \
   /*  106  */ 27643,-7539, \
   /*  107  */ 12565,-7539, \
   /*  108  */ 2513,-12565, \
   /*  109  */ 22617,-7539, \
   /*  110  */ 12565,-12565, \
   /*  111  */ 7539,-12565, \
   /*  112  */ 22617,-12565, \
   /*  113  */ 17591,-12565, \
   /*  114  */ 7539,-17591, \
   /*  115  */ 27643,-12565, \
   /*  116  */ 17591,-17591, \
   /*  117  */ 2513,-17591, \
   /*  118  */ 27643,-17591, \
   /*  119  */ 12565,-17591, \
   /*  120  */ 2513,-22617, \
   /*  121  */ 22617,-17591, \
   /*  122  */ 12565,-22617, \
   /*  123  */ 7539,-22617, \
   /*  124  */ 22617,-22617, \
   /*  125  */ 17591,-22617, \
   /*  126  */ 7539,-27643, \
   /*  127  */ 2513,-27643, \
};


const int16_t OFDM_256QAM_MAP[512] = { \
   /*   0   */ 1849,1849, \
   /*   1   */ 5547,1849, \
   /*   2   */ 9245,1849, \
   /*   3   */ 12943,1849, \
   /*   4   */ 16641,1849, \
   /*   5   */ 20339,1849, \
   /*   6   */ 24037,1849, \
   /*   7   */ 27735,1849, \
   /*   8   */ 31433,1849, \
   /*   9   */ 1849,5547, \
   /*  10   */ 5547,5547, \
   /*  11   */ 9245,5547, \
   /*  12   */ 12943,5547, \
   /*  13   */ 16641,5547, \
   /*  14   */ 20339,5547, \
   /*  15   */ 24037,5547, \
   /*  16   */ 27735,5547, \
   /*  17   */ 31433,5547, \
   /*  18   */ 1849,9245, \
   /*  19   */ 5547,9245, \
   /*  20   */ 9245,9245, \
   /*  21   */ 12943,9245, \
   /*  22   */ 16641,9245, \
   /*  23   */ 20339,9245, \
   /*  24   */ 24037,9245, \
   /*  25   */ 27735,9245, \
   /*  26   */ 31433,9245, \
   /*  27   */ 1849,12943, \
   /*  28   */ 5547,12943, \
   /*  29   */ 9245,12943, \
   /*  30   */ 12943,12943, \
   /*  31   */ 16641,12943, \
   /*  32   */ 20339,12943, \
   /*  33   */ 24037,12943, \
   /*  34   */ 27735,12943, \
   /*  35   */ 5547,16641, \
   /*  36   */ 1849,16641, \
   /*  37   */ 12943,16641, \
   /*  38   */ 9245,16641, \
   /*  39   */ 20339,16641, \
   /*  40   */ 16641,16641, \
   /*  41   */ 27735,16641, \
   /*  42   */ 24037,16641, \
   /*  43   */ 1849,20339, \
   /*  44   */ 5547,20339, \
   /*  45   */ 9245,20339, \
   /*  46   */ 12943,20339, \
   /*  47   */ 16641,20339, \
   /*  48   */ 20339,20339, \
   /*  49   */ 24037,20339, \
   /*  50   */ 1849,24037, \
   /*  51   */ 5547,24037, \
   /*  52   */ 9245,24037, \
   /*  53   */ 12943,24037, \
   /*  54   */ 16641,24037, \
   /*  55   */ 20339,24037, \
   /*  56   */ 5547,27735, \
   /*  57   */ 1849,27735, \
   /*  58   */ 12943,27735, \
   /*  59   */ 9245,27735, \
   /*  60   */ 1849,31433, \
   /*  61   */ 16641,27735, \
   /*  62   */ 9245,31433, \
   /*  63   */ 5547,31433, \
   /*  64   */ -5547,1849, \
   /*  65   */ -1849,1849, \
   /*  66   */ -12943,1849, \
   /*  67   */ -9245,1849, \
   /*  68   */ -20339,1849, \
   /*  69   */ -16641,1849, \
   /*  70   */ -27735,1849, \
   /*  71   */ -24037,1849, \
   /*  72   */ -1849,5547, \
   /*  73   */ -31433,1849, \
   /*  74   */ -9245,5547, \
   /*  75   */ -5547,5547, \
   /*  76   */ -16641,5547, \
   /*  77   */ -12943,5547, \
   /*  78   */ -24037,5547, \
   /*  79   */ -20339,5547, \
   /*  80   */ -31433,5547, \
   /*  81   */ -27735,5547, \
   /*  82   */ -5547,9245, \
   /*  83   */ -1849,9245, \
   /*  84   */ -12943,9245, \
   /*  85   */ -9245,9245, \
   /*  86   */ -20339,9245, \
   /*  87   */ -16641,9245, \
   /*  88   */ -27735,9245, \
   /*  89   */ -24037,9245, \
   /*  90   */ -1849,12943, \
   /*  91   */ -31433,9245, \
   /*  92   */ -9245,12943, \
   /*  93   */ -5547,12943, \
   /*  94   */ -16641,12943, \
   /*  95   */ -12943,12943, \
   /*  96   */ -24037,12943, \
   /*  97   */ -20339,12943, \
   /*  98   */ -5547,16641, \
   /*  99   */ -27735,12943, \
   /*  100  */ -12943,16641, \
   /*  101  */ -1849,16641, \
   /*  102  */ -20339,16641, \
   /*  103  */ -9245,16641, \
   /*  104  */ -27735,16641, \
   /*  105  */ -16641,16641, \
   /*  106  */ -1849,20339, \
   /*  107  */ -24037,16641, \
   /*  108  */ -9245,20339, \
   /*  109  */ -5547,20339, \
   /*  110  */ -16641,20339, \
   /*  111  */ -12943,20339, \
   /*  112  */ -24037,20339, \
   /*  113  */ -20339,20339, \
   /*  114  */ -5547,24037, \
   /*  115  */ -1849,24037, \
   /*  116  */ -12943,24037, \
   /*  117  */ -9245,24037, \
   /*  118  */ -20339,24037, \
   /*  119  */ -16641,24037, \
   /*  120  */ -1849,27735, \
   /*  121  */ -5547,27735, \
   /*  122  */ -9245,27735, \
   /*  123  */ -12943,27735, \
   /*  124  */ -16641,27735, \
   /*  125  */ -1849,31433, \
   /*  126  */ -5547,31433, \
   /*  127  */ -9245,31433, \
   /*  128  */ -1849,-1849, \
   /*  129  */ -5547,-1849, \
   /*  130  */ -9245,-1849, \
   /*  131  */ -12943,-1849, \
   /*  132  */ -16641,-1849, \
   /*  133  */ -20339,-1849, \
   /*  134  */ -24037,-1849, \
   /*  135  */ -27735,-1849, \
   /*  136  */ -31433,-1849, \
   /*  137  */ -1849,-5547, \
   /*  138  */ -5547,-5547, \
   /*  139  */ -9245,-5547, \
   /*  140  */ -12943,-5547, \
   /*  141  */ -16641,-5547, \
   /*  142  */ -20339,-5547, \
   /*  143  */ -24037,-5547, \
   /*  144  */ -27735,-5547, \
   /*  145  */ -31433,-5547, \
   /*  146  */ -1849,-9245, \
   /*  147  */ -5547,-9245, \
   /*  148  */ -9245,-9245, \
   /*  149  */ -12943,-9245, \
   /*  150  */ -16641,-9245, \
   /*  151  */ -20339,-9245, \
   /*  152  */ -24037,-9245, \
   /*  153  */ -27735,-9245, \
   /*  154  */ -31433,-9245, \
   /*  155  */ -1849,-12943, \
   /*  156  */ -5547,-12943, \
   /*  157  */ -9245,-12943, \
   /*  158  */ -12943,-12943, \
   /*  159  */ -16641,-12943, \
   /*  160  */ -20339,-12943, \
   /*  161  */ -24037,-12943, \
   /*  162  */ -27735,-12943, \
   /*  163  */ -5547,-16641, \
   /*  164  */ -1849,-16641, \
   /*  165  */ -12943,-16641, \
   /*  166  */ -9245,-16641, \
   /*  167  */ -20339,-16641, \
   /*  168  */ -16641,-16641, \
   /*  169  */ -27735,-16641, \
   /*  170  */ -24037,-16641, \
   /*  171  */ -1849,-20339, \
   /*  172  */ -5547,-20339, \
   /*  173  */ -9245,-20339, \
   /*  174  */ -12943,-20339, \
   /*  175  */ -16641,-20339, \
   /*  176  */ -20339,-20339, \
   /*  177  */ -24037,-20339, \
   /*  178  */ -1849,-24037, \
   /*  179  */ -5547,-24037, \
   /*  180  */ -9245,-24037, \
   /*  181  */ -12943,-24037, \
   /*  182  */ -16641,-24037, \
   /*  183  */ -20339,-24037, \
   /*  184  */ -5547,-27735, \
   /*  185  */ -1849,-27735, \
   /*  186  */ -12943,-27735, \
   /*  187  */ -9245,-27735, \
   /*  188  */ -1849,-31433, \
   /*  189  */ -16641,-27735, \
   /*  190  */ -9245,-31433, \
   /*  191  */ -5547,-31433, \
   /*  192  */ 5547,-1849, \
   /*  193  */ 1849,-1849, \
   /*  194  */ 12943,-1849, \
   /*  195  */ 9245,-1849, \
   /*  196  */ 20339,-1849, \
   /*  197  */ 16641,-1849, \
   /*  198  */ 27735,-1849, \
   /*  199  */ 24037,-1849, \
   /*  200  */ 1849,-5547, \
   /*  201  */ 31433,-1849, \
   /*  202  */ 9245,-5547, \
   /*  203  */ 5547,-5547, \
   /*  204  */ 16641,-5547, \
   /*  205  */ 12943,-5547, \
   /*  206  */ 24037,-5547, \
   /*  207  */ 20339,-5547, \
   /*  208  */ 31433,-5547, \
   /*  209  */ 27735,-5547, \
   /*  210  */ 5547,-9245, \
   /*  211  */ 1849,-9245, \
   /*  212  */ 12943,-9245, \
   /*  213  */ 9245,-9245, \
   /*  214  */ 20339,-9245, \
   /*  215  */ 16641,-9245, \
   /*  216  */ 27735,-9245, \
   /*  217  */ 24037,-9245, \
   /*  218  */ 1849,-12943, \
   /*  219  */ 31433,-9245, \
   /*  220  */ 9245,-12943, \
   /*  221  */ 5547,-12943, \
   /*  222  */ 16641,-12943, \
   /*  223  */ 12943,-12943, \
   /*  224  */ 24037,-12943, \
   /*  225  */ 20339,-12943, \
   /*  226  */ 5547,-16641, \
   /*  227  */ 27735,-12943, \
   /*  228  */ 12943,-16641, \
   /*  229  */ 1849,-16641, \
   /*  230  */ 20339,-16641, \
   /*  231  */ 9245,-16641, \
   /*  232  */ 27735,-16641, \
   /*  233  */ 16641,-16641, \
   /*  234  */ 1849,-20339, \
   /*  235  */ 24037,-16641, \
   /*  236  */ 9245,-20339, \
   /*  237  */ 5547,-20339, \
   /*  238  */ 16641,-20339, \
   /*  239  */ 12943,-20339, \
   /*  240  */ 24037,-20339, \
   /*  241  */ 20339,-20339, \
   /*  242  */ 5547,-24037, \
   /*  243  */ 1849,-24037, \
   /*  244  */ 12943,-24037, \
   /*  245  */ 9245,-24037, \
   /*  246  */ 20339,-24037, \
   /*  247  */ 16641,-24037, \
   /*  248  */ 1849,-27735, \
   /*  249  */ 5547,-27735, \
   /*  250  */ 9245,-27735, \
   /*  251  */ 12943,-27735, \
   /*  252  */ 16641,-27735, \
   /*  253  */ 1849,-31433, \
   /*  254  */ 5547,-31433, \
   /*  255  */ 9245,-31433, \
};

/**
  * @brief Plot an I/Q constellation point in the symbol buffer based on supplied data and constellation.
  * @param symbol_buffer base pointer to two registers ordered real, imaginary
  * @param data information to be modulated (read MSB first at full width of type)
  * @param constellation reference to constellation identifier 0-7
  * @param trellis if nonzero enables trellis coding
  * @retval none
  */
void OFDMModulate(int16_t *symbol_buffer, uint16_t data, int constellation, int trellis) {

    switch (constellation & OFDM_CONST_MASK) {
    case OFDM_CONST_BPSK:
        data = (data >> 14) & 0x2;
        symbol_buffer[0] = OFDM_BPSK_MAP[data];
        symbol_buffer[1] = OFDM_BPSK_MAP[data+1];
        break;
    case OFDM_CONST_QPSK:
        data = (data >> 13) & 0x6;
        symbol_buffer[0] = OFDM_QPSK_MAP[data];
        symbol_buffer[1] = OFDM_QPSK_MAP[data+1];
        break;
    case OFDM_CONST_8QAM:
        data = (data >> 12) & 0xE;
        symbol_buffer[0] = OFDM_8QAM_MAP[data];
        symbol_buffer[1] = OFDM_8QAM_MAP[data+1];
        //printf("[%6i + %6ij] from data %i\r\n", symbol_buffer[0], symbol_buffer[1], data>>1);
        break;
    case OFDM_CONST_16QAM:
        data = (data >> 11) & 0x1E;
        symbol_buffer[0] = OFDM_16QAM_MAP[data];
        symbol_buffer[1] = OFDM_16QAM_MAP[data+1];
        //printf("[%6i + %6ij] from data %x\r\n", symbol_buffer[0], symbol_buffer[1], data>>1);
        break;
    case OFDM_CONST_32QAM:
        data = (data >> 10) & 0x3E;
        symbol_buffer[0] = OFDM_32QAM_MAP[data];
        symbol_buffer[1] = OFDM_32QAM_MAP[data+1];
        //printf("[%6i + %6ij] from data %x\r\n", symbol_buffer[0], symbol_buffer[1], data>>1);
        break;
    case OFDM_CONST_64QAM:
        data = (data >> 9) & 0x7E;
        symbol_buffer[0] = OFDM_64QAM_MAP[data];
        symbol_buffer[1] = OFDM_64QAM_MAP[data+1];
        break;
    case OFDM_CONST_128QAM:
        data = (data >> 8) & 0xFE;
        symbol_buffer[0] = OFDM_128QAM_MAP[data];
        symbol_buffer[1] = OFDM_128QAM_MAP[data+1];
        break;
    case OFDM_CONST_256QAM:
        data = (data >> 7) & 0x1FE;
        symbol_buffer[0] = OFDM_256QAM_MAP[data];
        symbol_buffer[1] = OFDM_256QAM_MAP[data+1];
        break;
    }

}

void normalize_ifft_bw(OFDM_Tx_struct *tx) {
    // Calculate pointer to an offset above the memory where cyclic prefix will go later
    int16_t *offset_buffer = &tx->BasebandSamples[OFDM_CP_N];
   if (tx->Syncfield.Fields.Bandwidth > 0) {
      int32_t bw_gain = 32768;
      switch (tx->Syncfield.Fields.Bandwidth) {
         case OFDM_BW_FM_ENB:
            bw_gain = OFDM_BW1_IFFT_ATTEN;
            break;
         case OFDM_BW_FM_WB:
            bw_gain = OFDM_BW2_IFFT_ATTEN;
            break;
         case OFDM_BW_FM_EWB:
            bw_gain = OFDM_BW3_IFFT_ATTEN;
            break;
         default:
            break;
      }
      for (int i = 0; i < OFDM_FFT_N; i++) {
         // Apply fixed gain
         offset_buffer[i] <<= OFDM_IFFT_SHIFT;
         // Apply bandwidth-variable gain
         offset_buffer[i] = ((int32_t)offset_buffer[i] * bw_gain) >> 15;
      }
   }  else {
      // No bandwidth-variable gain for BW0
      for (int i = 0; i < OFDM_FFT_N; i++) {
         // Apply fixed gain
         offset_buffer[i] <<= OFDM_IFFT_SHIFT;
      }
   }
}

/**
  * @brief Generate an OFDM data symbol based on data contained in transmitter memory structure
  * @param tx pointer to transmitter memory structure
  * @retval number of time domain samples available in tx->BasebandSamples buffer
  */
int OFDMBuildDataSymbol1024(OFDM_Tx_struct *tx) {
    // Clear baseband sample buffer memory
    for (int i = 0; i < OFDM_FFT_N+OFDM_CP_N; i++) {
        tx->BasebandSamples[i] = 0;
    }
    // Calculate pointer to an offset above the memory where cyclic prefix will go later
    int16_t *offset_buffer = &tx->BasebandSamples[OFDM_CP_N];
    // Calculate how many bits can be placed in each subcarrier based on 
    // constellation complexity and Trellis Coding flag
    int effective_bits_per_symbol = (int)tx->Syncfield.Fields.Constellation + 1;
    if (tx->Syncfield.Fields.TrellisCode) {
        if (tx->Syncfield.Fields.Constellation > 0) {
            effective_bits_per_symbol--;
        }
    }
    // Allocate memory for decomposing the input bytes into 
    // constelation-sized bit chunks
    // Calculate the number of occupied subcarriers
    int start_bin = OFDM_Start_Bins[tx->Syncfield.Fields.Bandwidth];
    int end_bin = OFDM_End_Bins[tx->Syncfield.Fields.Bandwidth];
    int occupied_subcarrier_n = (end_bin - start_bin) + 1;
    for (int i = 0; i < occupied_subcarrier_n; i++) {
        int offset = (start_bin + i)<<1;
		// Check if this is a pilot or a data subcarrier
		int pilot = 0;
		for (int ii = 0; ii < OFDM_Pilot_N[tx->Syncfield.Fields.Bandwidth]; ii++) {
			if ((start_bin + i) == OFDM_Pilot_Bins[ii]) {
				pilot = 1;
			}
		}
        if (pilot) {
			// Put pilots in these carriers.
			offset_buffer[offset++] = OFDM_PILOT_I;
			offset_buffer[offset] = OFDM_PILOT_Q;
		} else {
			// Otherwise put data in these carriers.
            // First we need to ensure there are enough data bits in the WorkingWord to modulate one subcarrier symbol
			while (tx->WordBits < OFDM_CONST_MAX_BITS) {
				if (tx->PayloadByteIndex < tx->Syncfield.Fields.PayloadByteCount) {
					// Another byte remains.
                    // All the remaining bits (if any) in tx->WorkingWord are on the left side (MSB)
                    // Place the new data in the appropriate position by shifting left as required.
                    int16_t new_byte = ((int16_t)tx->InputData[tx->PayloadByteIndex++]) & 0xFF;
                    // Apply randomizer to this new byte of data
                    new_byte = new_byte ^ StepRandomizer(&tx->Randomizer, tx->Syncfield.Fields.RandomizerIndex);
                    //printf("new byte %4x, bit count: %i\r\n", new_byte, tx->WordBits);
                    new_byte <<= (8 - tx->WordBits);
					tx->WorkingWord |= new_byte;
					tx->WordBits += 8;
				} else {
                    // No data remains, insert pseudorandom data
					tx->WorkingWord |= StepRandomizer(&tx->Randomizer, tx->Syncfield.Fields.RandomizerIndex) << (8 - tx->WordBits);
					tx->WordBits += 8;
				}
			}
            //printf("working word: %4x, bit count: %i\r\n", tx->WorkingWord, tx->WordBits);
            //printf("mod details:\r\n");
            OFDMModulate(&offset_buffer[offset], tx->WorkingWord, tx->Syncfield.Fields.Constellation, tx->Syncfield.Fields.TrellisCode);
            // bits per subcarrier for selected waveform is constellation+1
			tx->WorkingWord<<=effective_bits_per_symbol;
			tx->WordBits-=effective_bits_per_symbol;
            tx->PayloadBits-=effective_bits_per_symbol;
            if (tx->PayloadBits < 0) {
                tx->PayloadBits = 0;
            }
        }
    }
    
    RIFFT(offset_buffer, OFDM_FFT_STAGE_N);

    normalize_ifft_bw(tx);

    // Create cyclic prefix by copying the end samples to the beginning.
    for (int i = 0; i < OFDM_CP_N; i++){
        tx->BasebandSamples[i] = tx->BasebandSamples[i+OFDM_FFT_N];
    }
    return(OFDM_FFT_N+OFDM_CP_N);
};


/**
  * @brief Calculate a CRC-8 for the syncfield using the CCITT recommended polynomial.
  * @param syncfield pointer to register containing sycnfield data
  * @retval none
  */
void OFDMCalcSyncCRC(uint32_t *syncfield) {
	// Initial conditions all 8 bits in consideration are set.
	// Treat the syncfield as a 32-bit value with the two high bits zero'd.
	uint32_t crc = 0;
    *syncfield = (*syncfield) & OFDM_SYNCFIELD_MASK;
	crc ^= *syncfield;
	uint32_t poly = (uint32_t)0x07<<(OFDM_SYNCWORD_DATA_N - 8); // Polynomial x^8 + x^2 + x + 1
	uint32_t mask = (uint32_t)1<<(OFDM_SYNCWORD_DATA_N - 1);
	for (int i = 0; i < OFDM_SYNCWORD_DATA_N - 8; i++) {
		if (crc & mask) {
			crc <<= 1;
			crc ^= poly; 
		} else {
			crc <<= 1;
		}
	}
	*syncfield = (*syncfield) | ((crc>>(OFDM_SYNCWORD_DATA_N - 8))&0xFF);
}

/**
  * @brief Generate a two-tone preamble or Schmidl-Cox sync symbol based on data contained in transmitter memory structure
  * @param tx pointer to transmitter memory structure
  * @retval number of time domain samples available in tx->BasebandSamples buffer
  */
int OFDMBuildSyncSymbol1024(OFDM_Tx_struct *tx) {
   if (tx->TxPreambleCount > 1) {
      tx->TxPreambleCount--;
      for (int i = 0; i < OFDM_PREAMBLE_SAMPLE_COUNT; i++) {
         tx->NCO1Phase = CalcPhaseAdvance(tx->NCO1Phase, (OFDM_PREAMBLE_TONE_1 * DSP_NORM_FREQ) / OFDM_FFT_SAMPLE_RATE);
         tx->NCO2Phase = CalcPhaseAdvance(tx->NCO2Phase, (OFDM_PREAMBLE_TONE_2 * DSP_NORM_FREQ) / OFDM_FFT_SAMPLE_RATE);
         tx->BasebandSamples[i] = ((GetSinSample(tx->NCO1Phase)>>1) + (GetSinSample(tx->NCO2Phase)>>1))>>OFDM_PRESYNC_SHIFT;
      }
      return OFDM_PREAMBLE_SAMPLE_COUNT;
   } else {
      tx->TxPreambleCount--;
      tx->NCO1Phase = 0;
      tx->NCO2Phase = 0;
      // Clear baseband sample buffer memory
      for (int i = 0; i < OFDM_FFT_N+OFDM_CP_N; i++) {
        tx->BasebandSamples[i] = 0;
      }
      int16_t *offset_buffer = &tx->BasebandSamples[OFDM_CP_N];
      // Put known data on the even-bin subcarriers.
      // Exclude DC (bin 0).
      int bit_index = 0;
      int word_index = 0;
      uint16_t shift_register = OFDM_Syncword[word_index++];
      // Data in the offset buffer is ordered as real, imaginary values interleaved.
      // Real values are in even indices, imaginary in odd indicies.
      // So we count by 2 in order to modulate the real parts only.
      // We are also counting by 2 again because only the even bins are modulated
      // in the syncword to create a 2-periodic symbol.
      for (int i = 4; i < OFDM_FFT_N; i+=4) {
        if (shift_register & 0x8000) {
            offset_buffer[i] = 32767;
        } else {
            offset_buffer[i] = -32767;
        }
        shift_register <<= 1;
        bit_index++;
        if (bit_index == 16) {
            bit_index = 0;
            if (word_index < 16) {
                shift_register = OFDM_Syncword[word_index++];
            }
        }
      }
      // Form the sync data field
      OFDMCalcSyncCRC(&tx->Syncfield.Word);
      uint32_t data = tx->Syncfield.Word;
      // Put unknown data in designated bins.
      for (int i = 0; i < OFDM_SYNCWORD_DATA_N; i++) {
        if (data & ((uint32_t)1<<(OFDM_SYNCWORD_DATA_N-1))) {
            // Consume the data word MSB first, starting at bit OFDM_SYNCWORD_DATA_N.
            // If this bit is set, invert the phase of this subcarrier.
            // offset_buffer index is doubled because imaginary values are interleaved
            // and only the real values are modulated in the syncword.
            offset_buffer[OFDM_SYNCFIELD_BIN(i) << 1] *= -1;
        }
        data <<= 1;
      }

      // Mute bins not used in this bandwidth
      int start_bin = OFDM_Start_Bins[tx->Syncfield.Fields.Bandwidth]<<1; // Doubled because i/q pairs stored sequentially
      for (int i = 0; i < start_bin; i++) {
        offset_buffer[i] = 0;
      }
      int end_bin = OFDM_End_Bins[tx->Syncfield.Fields.Bandwidth]<<1;
      for (int i = end_bin+2; i < OFDM_FFT_N; i++) {
        offset_buffer[i] = 0;
      }

      RIFFT(offset_buffer, OFDM_FFT_STAGE_N);

      normalize_ifft_bw(tx);

      // Create cyclic prefix by copying the end samples to the beginning.
      for (int i = 0; i < OFDM_CP_N; i++){
        tx->BasebandSamples[i] = tx->BasebandSamples[i+OFDM_FFT_N];
      }
      return(OFDM_FFT_N+OFDM_CP_N);
   }
};

/**
  * @brief Initialize the receiver memory structure
  * @param rx pointer to receiver memory structure
  * @retval none
  */
void OFDMRxInit(OFDM_Rx_struct *rx) {
    rx->State = OFDM_RX_SEARCH;
    rx->CircIndex1 = 0;
    rx->CircN1 = OFDM_BUF_N;
    rx->CircIndex2 = 0;
    rx->CircN2 = OFDM_BUF2_N;
	for (int i = 0; i < rx->CircN1; i++) {
		rx->CircBuf1[i] = 0;
	}
    for (int i = 0; i < rx->CircN2; i++) {
        rx->CircBuf2[i] = 0;
    }
    for (int i = 0; i < OFDM_FFT_N>>1; i++) {
        rx->P1CircBuf[i] = 0;
        rx->ECircBuf[i] = 0;
    }
    rx->P1Index = 0;
    rx->SyncInhibitTimer = 0;
    rx->SyncArm = 0;
    rx->P1 = 0;
    rx->E = 0;

    GenLPFIR2(rx->SyncLPF, OFDM_SC_P1_CUTOFF, OFDM_FFT_SAMPLE_RATE, rx->CircN2, 1);
}



/**
  * @brief Perform modified Schmidl-Cox sync detection upon samples in circular buffer
  * @param rx pointer to receiver memory structure
  * @retval SyncResult sample count from current circular buffer index to sync detect point in history (0 if none)
  */
int16_t DetectSC_5(OFDM_Rx_struct *rx) {
    int norm_shift = (OFDM_FFT_STAGE_N+OFDM_SC_REG_BITS)-1;
    // Schmidl-Cox sync detection is founded on autocorrelation of the signal series
    // on a time-delay of 1/2 symbol. The sync symbol is intentionally 2-periodic, so
    // it repeats once in its symbol time.
    // In an integer-math implementation, special attention needs to be paid to the 
    // scaling factor to prevent saturation of the result. Scaling factor (norm_shift) 
    // is related to the signal amplitude at this point (set by external AGC or internal
    // software steps prior to this point), as well as the number of MAC operations performed
    // for each value of p1. 


    // Calculate a new sample-by-sample value of P1
    // First, calculate the sample index 1/2 symbol in history
    int j = rx->CircIndex1 - (OFDM_FFT_N>>1);
    if (j < 0) {
        j += rx->CircN1;
    }
    // Update the P1 circular buffer index
    rx->P1Index++;
    if (rx->P1Index >= (OFDM_FFT_N>>1)) {
        rx->P1Index = 0;
    }
    // Drop off the last partial P1 value
    rx->P1 -= rx->P1CircBuf[rx->P1Index];
    // Calculate a new partial P1 value
    rx->P1CircBuf[rx->P1Index] = ((int32_t)rx->CircBuf1[rx->CircIndex1] * (int32_t)rx->CircBuf1[j]) >> norm_shift;
    // Add new partial value to P1 metric accumulator
    rx->P1 += rx->P1CircBuf[rx->P1Index];

    // Calculate a new sample-by-sample value of E
    // Drop off the last partial E value
    rx->E -= rx->ECircBuf[rx->P1Index];
    // Calculate a new partial E value
    rx->ECircBuf[rx->P1Index] = ((int32_t)rx->CircBuf1[rx->CircIndex1] * (int32_t)rx->CircBuf1[rx->CircIndex1]) >> norm_shift;
    // Add new partial value to E metric accumulator
    rx->E += rx->ECircBuf[rx->P1Index];



    // Apply low-pass filter to P1
    rx->CircIndex2++;
    if (rx->CircIndex2 >= rx->CircN2) {
        rx->CircIndex2 = 0;
    }
    rx->CircBuf2[rx->CircIndex2] = rx->P1;
    rx->P1MA = GetFastFilterOutput(rx->CircBuf2, rx->CircIndex2, rx->CircN2, rx->SyncLPF, rx->CircN2, 0);


    // Sync state machine
    rx->SyncResult = 0;
    switch (rx->SyncArm) {
    case 1:
        if (rx->SyncDelay >= OFDM_BUF_N) {
            // Abort sync search without result
            rx->SyncArm = 0;
        }
        // Check for reasons to end peak search.
        if (rx->P1MA < rx->SyncArmEnergy>>1) { // First, P1MA has dropped to less than half the energy at time of SyncArm
            rx->SyncArm = 0;
            rx->Delay = OFDM_FFT_N + rx->SyncDelay + (OFDM_BUF2_N>>1);
            rx->SyncResult = 1;
        }
        if (rx->P1MA > rx->P1Max) {
            rx->P1Max = rx->P1MA;
            rx->SyncDelay = 0;
        } else {
            rx->SyncDelay++;
        }
        break;
    case 0:
    default:
        rx->SyncInhibitTimer++;
        if (rx->SyncInhibitTimer > (OFDM_FFT_N>>2)) {
            rx->SyncInhibitTimer = OFDM_FFT_N>>2;
            if (rx->P1MA > rx->E>>1) {
                rx->SyncArm = 1;
                rx->SyncArmEnergy = rx->E;
                rx->P1Max = 0;
                rx->SyncDelay = 0;
                rx->SyncInhibitTimer = 0;
            }
        }
        break;
    }

    return rx->SyncResult;
}

/**
  * @brief Calculate a complex equalizer tap based on sync symbol reference subcarrier
  * @param ref pointer to complex sync symbol reference subcarrier ordered real, imaginary
  * @param tap pointer to base index of two memory registers where complex tap value will be stored
  * @param norm normalizing value to account for difference in gain between sync symbol and data symbols
  * @retval SyncResult sample count from current circular buffer index to sync detect point in history (0 if none)
  */
void CalcEqTap(int16_t *ref, int16_t *tap) {
    // Calculate abs squared of this complex tap.
    // tap pointer to real value, imaginary at +1
    int32_t real = ref[0];
    int32_t imag = ref[1];
    // Equalizer is designed to normalize the unit circle at magnitude 16384
    // so a 14 bit shift is used below.
    // This leaves headroom additive noise in the constellation without
    // saturating int16_t.
    int32_t mag = ((real * real) >> 14) + ((imag * imag) >> 14);
    if (mag > 0) {
        real = (real<<OFDM_CONST_SHIFT) / mag;
        imag = (imag<<OFDM_CONST_SHIFT) / mag;
        tap[0] = (int16_t)real;
        // Negate imaginary part to take conjugate.
        tap[1] = -(int16_t)imag;
    } else {
        tap[0] = 0;
        tap[1] = 0;
    }
}

/**
  * @brief Apply subcarrier-based constellation rotation to adjust apparent sample time offset
  * @note ComplexRotate_q15 assumes FFT1024
  * @param rx pointer to receiver memory structure
  * @param sto_adj adjustment increment in units of samples
  * @retval none
  */
void OFDMAdjSTO(OFDM_Rx_struct *rx, int sto_adj) {
    for (int i = 0; i < OFDM_FFT_N>>1; i++) {
        int k = sto_adj * i;
        ComplexRotate_q15(&rx->FFTBuf[i<<1], k);
    }
}

/**
  * @brief Calculate full spectrum equalizer based on detected Schmidl-Cox preamble sync symbol
  * @note also decodes BPSK syncfield carried in the sync symbol thru Euclidian Distance hypothesis against surrounding subcarriers
  * @param rx pointer to receiver memory structure
  * @retval none
  */
void OFDMCalcEq_2(OFDM_Rx_struct *rx) {
    // This routine is called when CircIndex1 is pointing to the last sample in the sync symbol.
    // Extract audio samples starting at the estimated center of the cyclic prefix.
    rx->SyncOffset = rx->CircIndex1; // Index to last sample added to circular buffer
    rx->SyncOffset -= rx->Delay;
    if (rx->SyncOffset < 0) {
        rx->SyncOffset += rx->CircN1;
    }
    int copy_index = rx->SyncOffset;
    for (int i = 0; i < OFDM_FFT_N; i++) {
        // *** Important *** if using integer FFT, ensure intermediate steps do not overflow
        // by adjusting the gain in the line below. Once AGC gains are set somewhere prior
        // in the processing chain, this gain value can remain fixed.
        rx->FFTBuf[i] = rx->CircBuf1[copy_index++]>>OFDM_FFT_SHIFT;
        if (copy_index >= rx->CircN1) {
            copy_index = 0;
        }
    }
    // Perform a real FFT of the audio data starting at a sample in the cyclic predfix,
    // as identified by the sync search.
    // Since we are working with real audio samples, we only need one side of the full
    // FFT spectrum. The RFFT function call generates OFDM_FFT_N/2 complex value parts,
    // which occupy OFDM_FFT_N memory locations. The DC bin is offset zero.
    RFFT(rx->FFTBuf, OFDM_FFT_STAGE_N);

    OFDMAdjSTO(rx, STO_ADJ);

    for(int i = 0; i < OFDM_FFT_N; i++) {
        // Set equalizer taps to zero.
        // Equalizer is ordered real, imaginary pair per sample.
        // There are OFDM_FFT_N / 2 pairs in the equalizer, because we only operate
        // on the positive side of the spectrum.
        rx->Eq[i] = 0;
    }

    int word_index = 0;
    int bit_index = 0;
    uint16_t shift_register = OFDM_Syncword[word_index++];
    // Skip the DC bin.
    // This could be optimized by starting and stopping at occupied bins.
    for (int i = 2; i < OFDM_FFT_N; i+=2) {
        int bin_index = i >> 1;
        if (bin_index % 2 == 0) {
            // This is an even bin, it contains modulation.
            // First calculate an equalizer tap.
            CalcEqTap(&rx->FFTBuf[i], &rx->Eq[i]);
            if ((shift_register & 0x8000) == 0) {
                // If a bit is set in the known data, the phase for that subcarrier was reversed before transmission.
                rx->Eq[i] = -rx->Eq[i];
                rx->Eq[i+1] = -rx->Eq[i+1];
            }
            shift_register <<= 1;
            bit_index++;
            if (bit_index == 16) {
                bit_index = 0;
                if (word_index < 16) {
                    shift_register = OFDM_Syncword[word_index++];
                }
            }
        } 
    }

    rx->Syncfield.Word = 0;
    // Now go thru the list of bins with uknown data and test bit hypotheses.
    for (int i = 0; i < OFDM_SYNCWORD_DATA_N; i++) {
        int bin_index = OFDM_SYNCFIELD_BIN(i);
        int j = bin_index << 1;
        int16_t htap_1[2];
        htap_1[0] = -rx->Eq[j];
        htap_1[1] = -rx->Eq[j+1];
        // Calculate the euclidian distance to the two surrounding equalizer tap values, which lack unknown data
        int32_t hsum_1 = (int32_t)EuclidianDistance_q15(htap_1, &rx->Eq[j-4]);
        hsum_1 += (int32_t)EuclidianDistance_q15(htap_1, &rx->Eq[j+4]);
        int32_t hsum_0 = (int32_t)EuclidianDistance_q15(&rx->Eq[j], &rx->Eq[j-4]);
        hsum_0 += (int32_t)EuclidianDistance_q15(&rx->Eq[j], &rx->Eq[j+4]);
        rx->Syncfield.Word<<=1;
        //printf("[%i, %i]", hsum_1, hsum_0);
        if (hsum_1 < hsum_0) {
            // Hypothesis 1 wins
            // This subcarrier was probably phase-reversed before transmission by unknown data.
            // Save the phase-corrected equalizer value.
            rx->Eq[j] = htap_1[0];
            rx->Eq[j+1] = htap_1[1];
            // Set a bit in the recovered unknown data.
            rx->Syncfield.Word |= 1;
        }
    }
	
    // Now interpolate equalizer tap values for the odd bins
    for (int i = 1; i < (OFDM_FFT_N-1)>>1; i+=2) {
        int j = i << 1;
        // Calculate real part
        // To prevent overflow in fixed integer width, the surrounding equalizer
        // tap values are halved before addition.
        rx->Eq[j] = (rx->Eq[j+2]>>1) + (rx->Eq[j-2]>>1);
        j++;
        // Calculate imaginary part
        rx->Eq[j] = (rx->Eq[j+2]>>1) + (rx->Eq[j-2]>>1);
    }
}


/**
  * @brief Math accelerator for calc_pilot_angle. Replaces division with multiply-and-shift for digits 0-9.
  * @param divisor, from 0 to 9
  * @retval (2^15)/divisor clamped to int16_t
  */
const int16_t near_inverse[10] = { \
0,\
32767,\
16384,\
10923,\
8192,\
6554,\
5461,\
4681,\
4096,\
3641\
};

int16_t near_div(int32_t big, int16_t small) {
    if (small != 0) {
        if (big >= 0) {
            big = (big * (int32_t)near_inverse[small]) >> 15;
        } else {
            big = -((-big * (int32_t)near_inverse[small]) >> 15);
        }
        return (int16_t)big;
    } else {
        return(0);
    }
}

/**
  * @brief evaluate pilot subcarriers in this data symbol and calculate angle offset normalized to subcarrier
  * @param iq pointer to base index of buffer containing RFFT of this data symbol, ordered real, imaginary
  * @param bandwidth the bandwidth selector from the syncfield
  * @note this funciton is only used by OFDMPilotTiming()
  * @retval normalized radians per subcarrier, scaled 32768 = pi radians
  */
int16_t calc_pilot_angle(int16_t *iq, int16_t bandwidth) {
    // Calculate sample time offset for each carrier
    int16_t ofdm_pilot_n = OFDM_Pilot_N[bandwidth];
    int32_t angle_sum = 0;
	int16_t angle_mem[ofdm_pilot_n];
    for (int i = 0; i < ofdm_pilot_n; i++) {
        int bin_i = OFDM_Pilot_Bins[i] << 1;
        int16_t angle = ApproxATan2_q15(iq[bin_i], iq[bin_i+1]);
        // Normalize angle to bin number
        angle = ((int32_t)angle * (int32_t)OFDM_Pilot_Bin_Inv[i]) >> 15;
		angle_mem[i] = angle;
        angle_sum += (int32_t)angle;
        //printf("pilot %i:%i+%ij, %i |  ", i, iq[bin_i], iq[bin_i+1], angle);
    }
    //angle_sum /= ofdm_pilot_n;
    angle_sum = near_div(angle_sum, ofdm_pilot_n);
    //printf("\r\nAverage angle per bin: %i\r\n", angle_sum);
	if (0) {
    	// Exclude the subcarrier most distant from the mean.
    	int16_t distances[ofdm_pilot_n];
    	for (int i = 0; i < ofdm_pilot_n; i++) {
    		if (angle_mem[i] > angle_sum) {
    			distances[i] = angle_mem[i] - angle_sum;
    		} else {
    			distances[i] = angle_sum - angle_mem[i];
    		}
    	}
    	int ex_i = 0;
    	int16_t max = 0;
    	for (int i = 0; i < ofdm_pilot_n; i++) {
    		if (distances[i] > max) {
    			ex_i = i;
    			max = distances[i];
    		}
    	}
    	// calculate the angle sum without outlier
    	angle_sum = 0;
    	for (int i = 0; i < ofdm_pilot_n; i++) {
    		if (i != ex_i) {
    			angle_sum += angle_mem[i];
    		}
    	}
    	//angle_sum /= (ofdm_pilot_n - 1);
        angle_sum = near_div(angle_sum, ofdm_pilot_n-1);
    }
    //printf("Angle division: %i\r\n", angle_sum);
	return (int16_t)angle_sum;
}


/**
  * @brief evaluate pilot subcarriers in this data symbol determine and remove sample time offset
  * @param rx pointer to receiver memory structure
  * @retval none
  */
void OFDMPilotTiming(OFDM_Rx_struct *rx) {
    // Analyze pilot subcarriers and apply rotation to each
    // subcarrier as required.
    // This function assumes the pilot tones have been equalized
    // to magnitude 16384, and all pilot tones have zero phase
    // at transmission. 
    
	// angle_err is in units of radians scaled so 32768 = pi
	int16_t angle_err = calc_pilot_angle(rx->FFTBuf, rx->Syncfield.Fields.Bandwidth);
	
	//printf("Uncorrected pilot angle err %i\r\n", angle_err);
    if (angle_err != 0) {
        for (int i = 0; i < OFDM_FFT_N>>1; i++) {
            int bin_i = i<<1;
            int16_t rotation = (angle_err * (int32_t)i) >> 6;
            ComplexRotate_q15(&rx->FFTBuf[bin_i], rotation);
        }
    }
	
	rx->AppliedPilotAngle = angle_err;
}

/**
  * @brief Calculate Euclidean Distance between two points.  
  * @param pointers to two real, imaginary pair values. Second value is halved before calc.
  * @retval Distance between points.
  */
int16_t ofdm_demod_ed(int16_t *a, int16_t *b) {
    int32_t real = (((int32_t)b[0]>>1) - (int32_t)a[0])>>1;
    int32_t imag = (((int32_t)b[1]>>1) - (int32_t)a[1])>>1;
    real = (real * real) >> 16;
    imag = (imag * imag) >> 16;
    return (real + imag);
}

/**
  * @brief Demodulate BPSK constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodEDBPSK(int16_t *iq) {
   uint16_t data = 0;
   if (ofdm_demod_ed(iq, (int16_t*)&OFDM_BPSK_MAP[0]) > ofdm_demod_ed(iq, (int16_t*)&OFDM_BPSK_MAP[1<<1])) {
        data = 1;
   }
   return data;
}


/**
  * @brief Demodulate QPSK constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodEDQPSK(int16_t *iq) {
   uint16_t data = 0;
   int16_t dist = 32767;
   for (uint16_t i = 0; i < 4; i++) {
      int16_t test_dist = ofdm_demod_ed(iq, (int16_t*)&OFDM_QPSK_MAP[i<<1]);
      if (test_dist < dist) {
         dist = test_dist;
         data = i;
      }
   }
   return data;
}

/**
  * @brief Demodulate 8PSK constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodED8QAM(int16_t *iq) {
   uint16_t data = 0;
   int16_t dist = 32767;
   for (uint16_t i = 0; i < 8; i++) {
      int16_t test_dist = ofdm_demod_ed(iq, (int16_t*)&OFDM_8QAM_MAP[i<<1]);
      if (test_dist < dist) {
         dist = test_dist;
         data = i;
      }
   }
   return data;
}

/**
  * @brief Demodulate 16QAM constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodED16QAM(int16_t *iq) {
   uint16_t data = 0;
   int16_t dist = 32767;
   for (uint16_t i = 0; i < 16; i++) {
      int16_t test_dist = ofdm_demod_ed(iq, (int16_t*)&OFDM_16QAM_MAP[i<<1]);
      if (test_dist < dist) {
         dist = test_dist;
         data = i;
      }
   }
   return data;
}

/**
  * @brief Demodulate 32QAM constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodED32QAM(int16_t *iq) {
   uint16_t data = 0;
   int16_t dist = 32767;
   for (uint16_t i = 0; i < 32; i++) {
      int16_t test_dist = ofdm_demod_ed(iq, (int16_t*)&OFDM_32QAM_MAP[i<<1]);
      if (test_dist < dist) {
         dist = test_dist;
         data = i;
      }
   }
   return data;
}

/**
  * @brief Demodulate 64QAM constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodED64QAM(int16_t *iq) {
   uint16_t data = 0;
   int16_t dist = 32767;
   for (uint16_t i = 0; i < 64; i++) {
      int16_t test_dist = ofdm_demod_ed(iq, (int16_t*)&OFDM_64QAM_MAP[i<<1]);
      if (test_dist < dist) {
         dist = test_dist;
         data = i;
      }
   }
   return data;
}

/**
  * @brief Demodulate 128QAM constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodED128QAM(int16_t *iq) {
   uint16_t data = 0;
   int16_t dist = 32767;
   for (uint16_t i = 0; i < 128; i++) {
      int16_t test_dist = ofdm_demod_ed(iq, (int16_t*)&OFDM_128QAM_MAP[i<<1]);
      if (test_dist < dist) {
         dist = test_dist;
         data = i;
      }
   }
   return data;
}

/**
  * @brief Demodulate 256QAM constellation using Euclidian Distance
  * @param iq pointer to base index of buffer containing single subcarrier iq
  * @retval demodulated binary data
  */
uint16_t OFDMDemodED256QAM(int16_t *iq) {
   uint16_t data = 0;
   int16_t dist = 32767;
   for (uint16_t i = 0; i < 256; i++) {
      int16_t test_dist = ofdm_demod_ed(iq, (int16_t*)&OFDM_256QAM_MAP[i<<1]);
      if (test_dist < dist) {
         dist = test_dist;
         data = i;
      }
   }
   return data;
}

/**
  * @brief Demodulate and de-randomize data symbol subcarrier constellations
  * @param rx pointer to receiver memory structure
  * @retval number of complete data bytes available in rx->DataBuf
  */
int OFDMDemodulate2(OFDM_Rx_struct *rx) {
   // Decode FFT output (interpret subcarrier constellations)
   // Return number of bytes decoded (depends on constellation)
   // rx->DemodWordBitCount must be zero'd right after sync detection
   // rx->DemodWord carries partial bytes across symbols.
   uint16_t (*demod[8])(int16_t*) = { \
      OFDMDemodEDBPSK, \
      OFDMDemodEDQPSK, \
      OFDMDemodED8QAM, \
      OFDMDemodED16QAM, \
      OFDMDemodED32QAM, \
      OFDMDemodED64QAM, \
      OFDMDemodED128QAM, \
      OFDMDemodED256QAM \
   };
   int start_bin = OFDM_Start_Bins[rx->Syncfield.Fields.Bandwidth];
   int end_bin = OFDM_End_Bins[rx->Syncfield.Fields.Bandwidth];
   int bits_per = rx->Syncfield.Fields.Constellation + 1;
   int byte_index = 0;
   for (int i = start_bin; i <= end_bin; i++) {
      int pilot = 0;
      for (int j = 0; j < OFDM_Pilot_N[rx->Syncfield.Fields.Bandwidth]; j++) {
         if (OFDM_Pilot_Bins[j] == i) {
            pilot = 1;
         }
      }
      if (pilot == 0) {
         rx->DemodWord <<= bits_per;
         rx->DemodWord |= demod[rx->Syncfield.Fields.Constellation](&rx->FFTBuf[i<<1]);
         rx->DemodWordBitCount += bits_per;
         if (rx->DemodWordBitCount >= 8) {
            rx->DataBuf[byte_index] = (uint8_t)((rx->DemodWord>>(rx->DemodWordBitCount - 8)) & 0xFF);
            //de-randomize
            rx->DataBuf[byte_index] = rx->DataBuf[byte_index] ^ StepRandomizer(&rx->Randomizer, rx->Syncfield.Fields.RandomizerIndex);
            rx->DemodWordBitCount -= 8;
            byte_index++;
         }
      }
   }
    return byte_index;
}

int16_t OFDMDetectPresync(OFDM_Rx_struct *rx) {
    RFFT(rx->FFTBuf, OFDM_FFT_STAGE_N);
    // Calculate the energy in bins 65 and 67
    int i = 65<<1;
    int32_t bin_65_e = ((int32_t)rx->FFTBuf[i] * (int32_t)rx->FFTBuf[i])>>15;
    i++;
    bin_65_e += ((int32_t)rx->FFTBuf[i] * (int32_t)rx->FFTBuf[i])>>15;
    i = 67<<1;
    int32_t bin_67_e = ((int32_t)rx->FFTBuf[i] * (int32_t)rx->FFTBuf[i])>>15;
    i++;
    bin_67_e += ((int32_t)rx->FFTBuf[i] * (int32_t)rx->FFTBuf[i])>>15;
    // Calculate the energy in the 16 bins on either side of the signal bins
    int32_t noise_e = 0;
    int j = 68<<1;
    for (int i = 0; i < 16; i++) {
        noise_e += ((int32_t)rx->FFTBuf[j] * (int32_t)rx->FFTBuf[j])>>15;
        j++;
        noise_e += ((int32_t)rx->FFTBuf[j] * (int32_t)rx->FFTBuf[j])>>15;
        j++;
    }
    j = (64-16) << 1;
    for (int i = 0; i < 16; i++) {
        noise_e += ((int32_t)rx->FFTBuf[j] * (int32_t)rx->FFTBuf[j])>>15;
        j++;
        noise_e += ((int32_t)rx->FFTBuf[j] * (int32_t)rx->FFTBuf[j])>>15;
        j++;
    }
    noise_e >>= 5;
    if (noise_e <1) {
        noise_e = 1;
    }
    bin_65_e = CalcDecibelEnergy(bin_65_e, noise_e);
    bin_67_e = CalcDecibelEnergy(bin_67_e, noise_e);
    if (bin_67_e > bin_65_e) {
        return (int16_t)bin_65_e;
    } else {
        return (int16_t)bin_67_e;
    }
}

/**
  * @brief Add a time domain sample to the circular buffer and search for sync
  * @param rx pointer to receiver memory structure
  * @param sample real-valued time domain sample
  * @retval sync_flag nonzero if sync detected
  */
int OFDMSyncSearch(OFDM_Rx_struct *rx, int16_t sample) {
    // Advance the circular buffer index.
    rx->CircIndex1++;
    if (rx->CircIndex1 >= rx->CircN1) {
        rx->CircIndex1 = 0;
    }
    // Place this sample in circular buffer
    rx->CircBuf1[rx->CircIndex1] = sample;
    // Check for preamble
    rx->PresyncIndex++;
    if (rx->PresyncIndex >= OFDM_FFT_N) {
        rx->PresyncIndex = 0;
        int copy_index = rx->CircIndex1 - OFDM_FFT_N;
        if (copy_index < 0) {
            copy_index += rx->CircN1;
        }
        for (int i = 0; i < OFDM_FFT_N; i++) {
            rx->FFTBuf[i] = rx->CircBuf1[copy_index++]>>OFDM_FFT_SHIFT;
        }
        rx->PresyncSNR = OFDMDetectPresync(rx);
        if (rx->PresyncSNR > 6) {
            rx->PresyncDetectFlag = 1;
        }
    }
    // Perform correlation
    int sync_flag = DetectSC_5(rx);

    return sync_flag;
}

/**
  * @brief Add a time domain sample to the circular buffer and decode a data symbol if enough samples have been collected
  * @param rx pointer to receiver memory structure
  * @param sample real-valued time domain sample
  * @retval number of complete data bytes available in rx->DataBuf
  */
int OFDMDataReceive(OFDM_Rx_struct *rx, int16_t sample) {
	// Collect and demodulate one data symbol.
    // Advance the circular buffer index.
    rx->CircIndex1++;
    if (rx->CircIndex1 >= rx->CircN1) {
        rx->CircIndex1 = 0;
    }
    // Place this sample in circular buffer
    rx->CircBuf1[rx->CircIndex1] = sample;

    rx->SampleCount++;
	// Check if we've collected enough samples to perform RFFT.
	if (rx->SampleCount >= OFDM_FFT_N + OFDM_CP_N) {
		rx->SampleCount = 0;
        // Advance SyncOffset one symbol.
        rx->SyncOffset += (OFDM_FFT_N + OFDM_CP_N);
        if (rx->SyncOffset >= rx->CircN1) {
            rx->SyncOffset -= rx->CircN1;
        }
        int copy_index = rx->SyncOffset;
        // Scale samples and copy into FFTBuf
        for(int i = 0; i < OFDM_FFT_N; i++) {
            rx->FFTBuf[i] = rx->CircBuf1[copy_index++]>>OFDM_FFT_SHIFT;
            if (copy_index >= rx->CircN1) {
                copy_index = 0;
            }
        }

		 RFFT(rx->FFTBuf, OFDM_FFT_STAGE_N);

        OFDMAdjSTO(rx, STO_ADJ);

        // Apply equalizer correction
        for (int i = 0; i < OFDM_FFT_N>>1; i++) {
            int16_t x[2];
            int j = i << 1;
            x[0] = rx->FFTBuf[j];
            x[1] = rx->FFTBuf[j+1];
            ComplexMul_var(&rx->Eq[j], x, &rx->FFTBuf[j], OFDM_CONST_SHIFT);
        }

        // Apply pilot tone constellation rotatoin
        OFDMPilotTiming(rx);

		// Return number of decoded bytes.
        int decoded_byte_count = OFDMDemodulate2(rx);
        rx->BytesRemaining -= decoded_byte_count;
        if (rx->BytesRemaining < 0) {
            decoded_byte_count += rx->BytesRemaining;
            rx->BytesRemaining = 0;
        }
		return decoded_byte_count;
	}

	return 0;
}

/**
  * @brief Calculate SNR of received SC preamble symbol
  * @note RFFT has already been performed
  * @param rx pointer to receiver memory structure
  * @retval none
  */
void OFDMCalcSNR(OFDM_Rx_struct *rx) {
    rx->SignalE = 0;
    rx->NoiseE = 0;
    for (int i = rx->FirstBin; i <= rx->LastBin; i++) {
        int ii = i<<1;
        if (i % 2 == 0) {
            // This is an even bin, it contains modulation.
            rx->SignalE += ((int32_t)rx->FFTBuf[ii] * (int32_t)rx->FFTBuf[ii])>>(15-OFDM_FFT_SHIFT);
            rx->SignalE += ((int32_t)rx->FFTBuf[ii+1] * (int32_t)rx->FFTBuf[ii+1])>>(15-OFDM_FFT_SHIFT);
        } else {
            // This is an odd bin, it contains noise.
            rx->NoiseE += ((int32_t)rx->FFTBuf[ii] * (int32_t)rx->FFTBuf[ii])>>(15-OFDM_FFT_SHIFT);
            rx->NoiseE += ((int32_t)rx->FFTBuf[ii+1] * (int32_t)rx->FFTBuf[ii+1])>>(15-OFDM_FFT_SHIFT);
        }
    }
    // SignalE contains Signal + Noise energy. Remove Noise energy:
    rx->SignalE -= rx->NoiseE;
    if (rx->NoiseE == 0) {
        rx->NoiseE = 1;
    }
}

/**
  * @brief Process received sync symbol: refine sync, calculate equalizer, decode syncfield, check CRC, calculate SNR
  * @note RFFT has already been performed
  * @param rx pointer to receiver memory structure
  * @retval none
  */
void OFDMProcessSyncSymbol(OFDM_Rx_struct *rx) {

    OFDMCalcEq_2(rx);

    // Reset the sample counter for determining timing of next FFT.
    rx->SampleCount = 0;
    
    // Setup the receiver to collect data samples. The data field in the sync symbol specifies:
    // Constellation
    // Randomizer selection
    // Waveform bandwidth

    uint32_t syncfield = rx->Syncfield.Word;
    OFDMCalcSyncCRC(&syncfield);
    if (syncfield == rx->Syncfield.Word) {
        rx->BytesRemaining = rx->Syncfield.Fields.PayloadByteCount;
        rx->ValidSyncCRC = 1;
        rx->FirstBin = OFDM_Start_Bins[rx->Syncfield.Fields.Bandwidth];
        rx->LastBin = OFDM_End_Bins[rx->Syncfield.Fields.Bandwidth];
        rx->ConstShift = OFDM_CONST_SHIFT;

    } else {
        rx->Syncfield.Word = 0;
        rx->BytesRemaining = 0;
        rx->ValidSyncCRC = 0;
    }
    OFDMCalcSNR(rx);
}

