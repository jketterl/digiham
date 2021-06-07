#include "crc.hpp"
#include <cstring>

using namespace Digiham::DStar;

bool Crc::isCrcValid(char* data, size_t len, uint16_t to_check) {
    uint16_t checksum = 0xFFFF;

    for (int k = 0; k < len; k++) {
        for (int i = 0; i < 8; i++) {
            checksum ^= (data[k] >> i) & 1;
            if (checksum & 1) {
                checksum = (checksum >> 1) ^ 0x8408;
            } else {
                checksum >>= 1;
            }
        }
    }

    // invert at the and
    checksum ^= 0xFFFF;

    return memcmp(&checksum, &to_check, 2) == 0;
}