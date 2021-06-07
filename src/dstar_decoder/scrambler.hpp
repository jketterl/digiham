#pragma once

#include <cstdlib>
#include <cstdint>

namespace Digiham::DStar {

    class Scrambler {
        public:
            void scramble(unsigned char* in, unsigned char* out, size_t length);
            void reset();
        private:
            uint8_t shift_register = 0b1111111;
    };

}