#include "dc_block.hpp"

#include <cmath>

using namespace Digiham::DcBlock;

#define R 0.998f
#define GAIN ((1 + R) / 2)

void DcBlock::process(float *input, float *output, size_t length) {
    for (size_t i = 0; i < length; i++) {
        // dc block filter implementation according to https://www.dsprelated.com/freebooks/filters/DC_Blocker.html
        float x = input[i];
        if (std::isnan(x)) x = 0.0f;
        float y = GAIN * (x - xm1) + R * ym1;
        xm1 = x;
        ym1 = y;
        output[i] = y;
    }
}
