#pragma once

namespace Digiham::Nxdn {

    class Sacch {
        public:
            static Sacch* parse(unsigned char* data);
        private:
            static void deinterleave(unsigned char* input, unsigned char* output);
            static unsigned int viterbi_decode(unsigned char* in, unsigned char* out);
            static const unsigned char trellis_transitions[16][2];
    };

}