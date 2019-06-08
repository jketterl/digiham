#include "crc16.h"

uint16_t crc16_checksum(uint32_t* data) {
    uint16_t checksum = 0;

    for (int i = 0; i < 32; i++) {
        // not sure if this should be reversed or not
        bool input = (*data >> (31 - i)) & 1;
        bool next_input = input ^ ((checksum >> 15) & 1);
        checksum <<= 1;
        checksum ^= (next_input << 12) | (next_input << 5) | next_input;
    }

    // invert at the and
    return checksum ^ 0xFFFF;
}

bool crc16(uint32_t* data, uint16_t* checksum) {
    return (*checksum == crc16_checksum(data));
}