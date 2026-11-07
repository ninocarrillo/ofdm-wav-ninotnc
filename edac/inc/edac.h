/* 
 * File:   EDAC.h
 * Author: nino
 *
 * Created on September 14, 2026, 3:57 PM
 */

#ifndef EDAC_H
#define	EDAC_H

#include "stdint.h"

#define EDAC_RS_GF_GENPOLY 285
#define EDAC_RS_FIRSTROOT 0

int EDACAppendCCITT16(uint8_t *, int);
uint16_t EDACGetCCITT16(uint8_t *, int);
int EDACCheckCCITT16(uint8_t *, int);
int EDACGetExtendedN(int, int, int);
int EDACEncodeRS2(uint8_t *, int, int, int);
int EDACDecodeRS2(uint8_t *, int, int, int, int*);

#endif	/* EDAC_H */

