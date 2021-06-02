#include "whitening.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

void decode_whitening(uint8_t* input, uint8_t* output, uint8_t num) {
    uint16_t wsr = 0b111001001;
    memset(output, 0, (num + 7) / 8);
    int i;
    for (i = 0; i < num; i++) {
        int pos = i / 8;
        int shift = 7 - i % 8;

        bool wb = wsr & 1;

        output[pos] |= (input[pos] & (1 << shift)) ^ (wb << shift);

        // shift and perform xor
        wb = ((wsr >> 4) & 1) ^ wb;
        wsr = ((wsr & 0b111111110) >> 1) | (wb << 8);
    }
}
