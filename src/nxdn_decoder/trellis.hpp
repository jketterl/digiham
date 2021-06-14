#pragma once

#include <cstdlib>
#include <cstdint>

namespace Digiham::Nxdn {

    typedef struct {
        uint16_t metric;
        uint8_t *data;
    } branch;

    class Trellis {
        public:
            unsigned int decode(unsigned char* input, unsigned char* output, size_t len);
        private:
            static const unsigned char transitions[16][2];
    };

}