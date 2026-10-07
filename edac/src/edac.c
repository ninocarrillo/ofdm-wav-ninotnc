#include "edac.h"
#include "crc.h"
#include "rs2.h"
#include "gf2.h"
#include "stdio.h"

int EDACAppendCCITT16(uint8_t *data, int count) {
    uint16_t crc = CCITT16CalcCRC(data, count);
    data[count++] = crc & 0xFF;
    data[count++] = (crc>>8) & 0xFF;
    return count;
}

uint16_t EDACGetCCITT16(uint8_t *data, int count) {
    uint16_t x, y;
    x = data[count-2];
    y = data[count-1] & 0xFF;
    x |= y << 8;
    return x;
}

int EDACCheckCCITT16(uint8_t *data, int count) {
    int result = 0;
    if (CCITT16CalcCRC(data, count-2) == EDACGetCCITT16(data, count)) {
        result = count - 2;
    }
    return result;
}


int EDACGetExtendedN(int payload_n, int sub_n, int parity_n) {
    int block_count = payload_n / sub_n;
    if ((block_count * sub_n) < payload_n) {
        block_count++;
    }
    return payload_n + (block_count * parity_n);
}

int EDACGetPayloadN(int extended_n, int sub_n, int parity_n) {
    int block_count = extended_n / (sub_n + parity_n);
    if (extended_n > (block_count * (sub_n + parity_n))) {
        block_count++;
    }
    return extended_n - (block_count * parity_n);
}

int edac_move_block_right(uint8_t *buf, int start, int offset, int count) {
    for (int i = 0; i < count; i++) {
        buf[start+offset] = buf[start];
        buf[start] = 0;
        start--;
    }
    return count;
}

int edac_move_block_left(uint8_t *buf, int start, int offset, int count) {
    for (int i = 0; i < count; i++) {
        buf[start] = buf[start+offset];
        buf[start+offset] = 0;
        start++;
    }
    return count;
}

int EDACEncodeRS2(uint8_t *data, int payload_n, int sub_n, int parity_n) {
    int block_count = payload_n / sub_n;
    if ((block_count * sub_n) < payload_n) {
        block_count++;
    }
    int small_n = payload_n / block_count;
    int big_count = payload_n - (small_n * block_count);
    int small_count = block_count - big_count;
    printf("payload_n: %i, sub_n: %i, parity_n: %i, block_count: %i, small_n: %i, big_count: %i\r\n", payload_n, sub_n, parity_n, block_count, small_n, big_count);


    GF2_def_struct gf;
    InitGF2(285, &gf);
    RS2_def_struct rs;
    rs.GF = &gf;
    InitRS2(0, parity_n, &rs);

    // extend the payload to make room for parity
    int read_i = payload_n - 1;
    int offset = parity_n * (block_count - 1);

    // Move operation count is one less than block_count.
    // big_count is always less than block_count.

    int move_count = 0;
    int move_n = small_n;
    for (int i = 0; i < block_count-1; i++) {
        read_i -= edac_move_block_right(data, read_i, offset, move_n);
        RSEncode(&data[read_i + offset + 1], move_n, &rs);

        offset -= parity_n;
        move_count++;
        if (move_count == small_count) {
            move_n++;
        }
    }
    RSEncode(&data[0], move_n, &rs);
    return payload_n + (block_count * parity_n);
}

int EDACDecodeRS2(uint8_t *data, int extended_n, int sub_n, int parity_n) {
    int block_count = extended_n / (sub_n + parity_n);
    if (extended_n > (block_count * (sub_n + parity_n))) {
        block_count++;
    }
    int payload_n = extended_n - (block_count * parity_n);
    int small_n = payload_n / block_count;
    int big_count = payload_n - (small_n * block_count);
    int small_count = block_count - big_count;
    printf("payload_n: %i, sub_n: %i, parity_n: %i, block_count: %i, small_n: %i, big_count: %i\r\n", payload_n, sub_n, parity_n, block_count, small_n, big_count);
    // Decode RS blocks here
    // Collapse payload
    // Start with big blocks



    GF2_def_struct gf;
    InitGF2(285, &gf);
    RS2_def_struct rs;
    rs.GF = &gf;
    InitRS2(0, parity_n, &rs);

    int offset = parity_n;

    // Move operation count is one less than block_count.
    // big_count is always less than block_count.

    int write_i = small_n;
    if (big_count > 0) {
        write_i++;
    }

    int total_corrected = 0;
    int uncorrected = 0;

    int corrected = RSDecode(&data[0], write_i+parity_n, &rs);
    if (corrected >= 0) {
        total_corrected += corrected;
    } else {
        uncorrected++;
    }

    int move_n = small_n;
    if (big_count > 1) {
        move_n++;
    }
    int move_count = 0;
    for (int i = 0; i < block_count-1; i++) {
        corrected = RSDecode(&data[write_i+offset], move_n+parity_n, &rs);
        if (corrected >= 0) {
            total_corrected += corrected;
        } else {
            uncorrected++;
        }
        write_i += edac_move_block_left(data, write_i, offset, move_n);
        offset += parity_n;
        move_count++;
        if ((move_count+1)==big_count) {
            move_n--;
        }
    }
    printf("Corrected %i bytes.\r\n", total_corrected);
    printf("Found %i uncorrectable blocks.\r\n", uncorrected);
    return payload_n;
}

