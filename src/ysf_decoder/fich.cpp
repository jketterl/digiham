#include <cstdint>
#include "fich.hpp"

extern "C" {
#include "trellis.h"
#include "golay_24_12.h"
#include "crc16.h"
}

using namespace Digiham::Ysf;

Fich *Fich::parse(unsigned char *data) {
    uint8_t fich_raw[25] = { 0 };

    // 5 by 20 deinterleave
    for (int i = 0; i < 100; i++) {
        int offset = ((i * 20) % 100 + i * 20 / 100);
        fich_raw[i / 4] |= (data[offset] & 3) << (6 - 2 * (i % 4));
    }

    uint8_t fich_trellis[13];
    uint8_t result = decode_trellis(&fich_raw[0], 100, &fich_trellis[0]);

    uint32_t fich_golay[4] = { 0 };
    bool golay_result = true;
    for (int i = 0; i < 4; i++) {
        fich_golay[i] = 0 |
                fich_trellis[i * 3] << 16 |
                fich_trellis[i * 3 + 1] << 8 |
                fich_trellis[i * 3 + 2];
        golay_result &= golay_24_12(&fich_golay[i]);
    }

    if (!golay_result) return nullptr;

    // re-combine final fich from golay result
    uint32_t fich_data = 0 |
            (fich_golay[0] & 0b00000000111111111111000000000000) << 8 |
            (fich_golay[1] & 0b00000000111111111111000000000000) >> 4 |
            (fich_golay[2] & 0b00000000111111110000000000000000) >> 16;

    uint16_t fich_checksum = 0 |
            (fich_golay[2] & 0b00000000000000001111000000000000) |
            (fich_golay[3] & 0b00000000111111111111000000000000) >> 12;

    // the endianess is reversed at this point for some reason.
    uint32_t be =  __builtin_bswap32(fich_data);

    if (!crc16((uint8_t*) &be, 4, &fich_checksum)) return nullptr;

    return new Fich(fich_data);
}

Fich::Fich(uint32_t data): data(data) {}

unsigned char Fich::getFrameType() const {
    return (data >> 30) & 0b11;
}

unsigned char Fich::getDataType() const {
    return (data >> 8) & 0b11;
}

unsigned char Fich::getFrameNumber() const {
    return (data >> 19) & 0b111;
}