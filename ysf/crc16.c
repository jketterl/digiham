#include "crc16.h"

uint16_t crc16_checksum(uint8_t* data, int count) {
    uint16_t checksum = 0;

    for (int k = 0; k < count; k++) {
        uint8_t* current_input = data + k;
        for (int i = 0; i < 8; i++) {
            bool input = (*current_input >> (7 - i)) & 1;
            bool next_input = input ^ ((checksum >> 15) & 1);
            checksum <<= 1;
            checksum ^= (next_input << 12) | (next_input << 5) | next_input;
        }
    }

    // invert at the and
    return checksum ^ 0xFFFF;
}

bool crc16(uint8_t* data, int count, uint16_t* checksum) {
    return (*checksum == crc16_checksum(data, count));
}