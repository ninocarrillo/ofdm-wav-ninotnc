#ifndef OFDM_RX_H
#define OFDM_RX_H

#define OFDM_RX_MAX_PAYLOAD 512
#define OFDM_RX_BUF_LEN (OFDM_RX_MAX_PAYLOAD + 16 + (OFDM_RX_MAX_PAYLOAD >> 3))
#define DECI_FIR_N 157
#define DECI_FIR_CUTOFF 2836
#define MAX_DECI_RATE 8

#endif