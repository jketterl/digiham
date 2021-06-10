#pragma once

#include <cstdlib>
#include <cstdint>

namespace Digiham::Nxdn {

    class Scrambler {
        public:
            void scramble(unsigned char* in, unsigned char* out, size_t length);
            void reset();
        private:
            uint16_t shift_register = 0b011100100;
    };

}