#pragma once

#include <string>

namespace Digiham::DStar {

    class Header {
        public:
            static Header* parse(char* raw);
            static const unsigned int bits = 660;

            Header(char* data);
            ~Header();
            bool isCrcValid();
            bool isData();
            bool isVoice() { return !isData(); }
            std::string getDestinationRepeater();
            std::string getDepartureRepeater();
            std::string getCompanion();
            std::string getOwnCallsign();
            std::string toString();
        private:
            static void deinterleave(char* in, char* out);
            static unsigned int viterbi_decode(char* in, char* out);
            static const unsigned char trellis_transitions[4][2];

            std::string rtrim(std::string input);

            char* data;
    };

}