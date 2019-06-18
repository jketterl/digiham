#include "bptc_196_96.h"
#include "hamming_13_9.h"
#include "hamming_15_11.h"

bool bptc_196_96(uint8_t* payload, uint8_t* output) {
    uint8_t i, k;

    // deinterleave payload according to ETSI B.1.1 (BPTC196,96)
    // Interleave Index = Index × 181 modulo 196

    uint8_t payload_deinterleaved[25] = { 0 };
    for (i = 0; i < 196; i++) {
        uint8_t source_index = (i * 181) % 196;
        payload_deinterleaved[i / 8] |= ((payload[source_index / 8] >> (7 - (source_index % 8))) & 1) << (7 - (i % 8));
    }

    // pivot the whole matrix in order to apply the column FEC
    uint16_t payload_pivoted[15] = { 0 };
    bool hamming_result = true;
    for (i = 0; i < 15; i++) {
        for (k = 0; k < 13; k++) {
            // there's only 195 bits in the matrix, but we received 196, that's fixed by the +1
            // that stupid R(3) bit has cost me lots of time
            uint8_t source_index = k * 15 + i + 1;
            payload_pivoted[i] |= ((payload_deinterleaved[source_index / 8] >> (7 - (source_index % 8))) & 1) << (12 - k);
        }
        hamming_result &= hamming_13_9(&payload_pivoted[i]);
    }

    if (!hamming_result) return false;

    // pivot back in order to apply row FEC (we can already drop the final rows at this point)
    uint16_t payload_rows[9] = { 0 };

    for (i = 0; i < 9; i++) {
        for (k = 0; k < 15; k++) {
            payload_rows[i] |= ((payload_pivoted[k] >> (12 - i)) & 1) << (14 - k);
        }
        hamming_result &= hamming_15_11(&payload_rows[i]);
    }

    if (!hamming_result) return false;

    // TODO CRC checksum is in the last 24 bits
    output[0]  =  (payload_rows[0] & 0b000111111110000) >> 4;
    output[1]  =  (payload_rows[1] & 0b111111110000000) >> 7;
    output[2]  = ((payload_rows[1] & 0b000000001110000) << 1) | ((payload_rows[2] & 0b111110000000000) >> 10);
    output[3]  = ((payload_rows[2] & 0b000001111110000) >> 2) | ((payload_rows[3] & 0b110000000000000) >> 13);
    output[4]  =  (payload_rows[3] & 0b001111111100000) >> 5;
    output[5]  = ((payload_rows[3] & 0b000000000010000) << 3) | ((payload_rows[4] & 0b111111100000000) >> 8);
    output[6]  =  (payload_rows[4] & 0b000000011110000)       | ((payload_rows[5] & 0b111100000000000) >> 11);
    output[7]  = ((payload_rows[5] & 0b000011111110000) >> 3) | ((payload_rows[6] & 0b100000000000000) >> 14);
    output[8]  =  (payload_rows[6] & 0b011111111000000) >> 6;
    output[9]  = ((payload_rows[6] & 0b000000000110000) << 2) | ((payload_rows[7] & 0b111111000000000) >> 9);
    output[10] = ((payload_rows[7] & 0b000000111110000) >> 1) | ((payload_rows[8] & 0b111000000000000) >> 12);
    output[11] = ((payload_rows[8] & 0b000111111110000) >> 4);

    return true;
}