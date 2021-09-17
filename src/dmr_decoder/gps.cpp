#include "gps.hpp"

#include <cstdint>

using namespace Digiham::Dmr;

Digiham::Coordinate* Gps::parse(const unsigned char* data) {
    int32_t latitudeBits = ((data[4] & 0b01111111) << 16) | (data[5] << 8) | data[6];
    if (data[4] & 0b10000000) latitudeBits *= -1;
    int32_t longitudeBits = (data[1] << 16) | (data[2] << 8) | data[3];
    if (data[0] & 0b00000001) longitudeBits *= -1;

    return new Digiham::Coordinate(
        180.0f / (float) (1 << 24) * (float)latitudeBits,
        360.0f / (float) (1 << 25) * (float)longitudeBits
    );
}