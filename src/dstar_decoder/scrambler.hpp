#pragma once

#include <cstdlib>
#include <cstdint>

namespace Digiham::DStar {

    class Scrambler {
        public:
            void scramble(char* in, char* out, size_t length);
            void reset();
        private:
            uint8_t shift_register = 0b1111111;
    };

}