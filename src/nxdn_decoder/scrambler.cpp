#include "scrambler.hpp"
#include <cstring>

#include <iostream>

using namespace Digiham::Nxdn;

void Scrambler::reset() {
    shift_register = 0b011100100;
}

void Scrambler::scramble(unsigned char* input, unsigned char* output, size_t len) {
    memset(output, 0, len);

    for (int i = 0; i < len; i++) {
        bool wb = shift_register & 1;
        // apply
        // according to the spec, the symbols shall be multiplied with -1 if the scrambler output is 1
        // in iffect, that's an XOR on the high symbol bit. go figure...
        output[i] = (input[i] & 3) ^ (wb << 1);

        // shift and perform xor
        wb = ((shift_register >> 4) & 1) ^ wb;
        shift_register = ((shift_register & 0b111111110) >> 1) | (wb << 8);
    }
}