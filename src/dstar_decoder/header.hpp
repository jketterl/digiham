#pragma once

namespace Digiham::DStar {

    class Header {
        public:
            static Header* parse(char* raw);
            static const unsigned int bits = 660;

            Header(char* data);
            ~Header();
            bool isData();
            bool isVoice() { return !isData(); }
        private:
            static void deinterleave(char* in, char* out);
            static unsigned int viterbi_decode(char* in, char* out);
            static const unsigned char trellis_transitions[4][2];

            char* data;
    };

}