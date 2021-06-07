#include "scrambler.hpp"
#include <cstring>

using namespace Digiham::DStar;

void Scrambler::reset() {
    shift_register = 0b1111111;
}

void Scrambler::scramble(unsigned char* input, unsigned char* output, size_t len) {
    memset(output, 0, len);
    for (int i = 0; i < len; i++) {
        // calculate whitening bit
        bool wb = (shift_register & 1) ^ ((shift_register >> 3) & 1);

        // apply
        output[i] = (input[i] & 1) ^ wb;

        // rotate
        shift_register = ((shift_register & 0b1111110) >> 1) | (wb << 6);
    }
}