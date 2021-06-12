#pragma once

namespace Digiham::Nxdn {

    class Sacch {
        public:
            static Sacch* parse(unsigned char* data);

            Sacch(unsigned char* data);
            ~Sacch();

            unsigned char getStructureIndex();
        private:
            static void deinterleave(unsigned char* input, unsigned char* output);
            static void inflate(unsigned char* input, unsigned char* output);
            static unsigned int viterbi_decode(unsigned char* in, unsigned char* out);
            static bool check_crc(unsigned char* in);
            static const unsigned char trellis_transitions[16][2];

            unsigned char* data;
    };

}