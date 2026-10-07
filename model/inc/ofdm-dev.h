/* 
 * File:   ofdm_dev.h
 * Author: ninoc
 *
 * Created on May 17, 2026, 8:03 AM
 */

#ifndef OFDM_DEV_H
#define	OFDM_DEV_H
#include "ofdm.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define OD_BUF_LEN ((OFDM_FFT_N + OFDM_CP_N) * 2)

typedef struct {
	int P1[OD_BUF_LEN];
	int P1MA[OD_BUF_LEN];
	int E[OD_BUF_LEN];
	int Index;
} OFDM_Dev_struct;

void OFDMWriteTitle(void *);
void OFDMDumpRX(OFDM_Rx_struct *, void *);
void OFDMDumpEq(OFDM_Rx_struct *, void *);
void OFDMDumpBaseband(OFDM_Rx_struct *, void *);
void OFDMDumpSync(OFDM_Rx_struct *, void *);
void OFDMEqSVG(OFDM_Rx_struct *, int);
void OFDMEqMagSVG(OFDM_Rx_struct *, int);
void OFDMSymbolSVG(OFDM_Rx_struct *, int, int);
void OFDMSyncDump(OFDM_Rx_struct *, OFDM_Dev_struct *);
void OFDMSyncSVG(OFDM_Rx_struct *, OFDM_Dev_struct *, int);

#ifdef	__cplusplus
}
#endif

#endif	/* OFDM_DEV_H */

