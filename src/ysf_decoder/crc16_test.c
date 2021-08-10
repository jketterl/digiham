#include "crc16.h"
#include <stdint.h>
#include <assert.h>

int main() {
    //uint8_t input[] = { 0x60, 0x06, 0x56, 0x00 };
    //uint16_t expected = 0xB094;
    //int count = 4;
    uint8_t input[] = { 0x44, 0x42, 0x30, 0x42, 0x5A, 0x41, 0x20, 0x20, 0x20, 0x20 };
    uint16_t expected = 0x9c1e;
    int count = 10;
    //uint8_t input[] = { 0x44, 0x42 };
    //uint16_t expected = 0x5671;
    //int count = 2;

    uint16_t calculated = crc16_checksum((uint8_t*) &input, count);

    assert(crc16((uint8_t*) &input, count, &expected));
}