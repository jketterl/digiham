#pragma once

#include <string>

namespace Digiham::DStar {

    class Header {
        public:
            static Header* parseFromHeader(unsigned char* raw);
            static Header* parseFromFrameData(unsigned char* data);
            static const unsigned int bits = 660;

            explicit Header(unsigned char* data);
            ~Header();
            bool isData();
            bool isVoice() { return !isData(); }
            std::string getDestinationRepeater();
            std::string getDepartureRepeater();
            std::string getCompanion();
            std::string getOwnCallsign();
            std::string toString();
        private:
            static void deinterleave(unsigned char* in, unsigned char* out);
            static unsigned int viterbi_decode(unsigned char* in, unsigned char* out);
            static const unsigned char trellis_transitions[4][2];

            std::string rtrim(std::string input);

            unsigned char* data;
    };

}