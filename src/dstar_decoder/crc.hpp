#pragma once

#include <cstdlib>
#include <cstdint>

namespace Digiham::DStar {

    class Crc {
        public:
            static bool isCrcValid(unsigned char* data, size_t len, uint16_t checksum);
    };

}