#pragma once

#include <cstdint>

#define SACCH_MESSAGE_TYPE_VCALL 0x01

// spec has more types, but i think that's all we're interested in right now
#define SACCH_CALL_TYPE_BROADCAST 0b000
#define SACCH_CALL_TYPE_CONFERENCE 0b001
#define SACCH_CALL_TYPE_INDIVIDUAL 0b100

namespace Digiham::Nxdn {

    class Sacch {
        public:
            static Sacch* parse(unsigned char* data);

            Sacch(unsigned char* data);
            ~Sacch();

            unsigned char getStructureIndex();
            unsigned char* getSuperframeData();
        private:
            static void deinterleave(unsigned char* input, unsigned char* output);
            static void inflate(unsigned char* input, unsigned char* output);
            static unsigned int viterbi_decode(unsigned char* in, unsigned char* out);
            static bool check_crc(unsigned char* in);
            static const unsigned char trellis_transitions[16][2];

            unsigned char* data;
    };

    class SacchSuperframe {
        public:
            SacchSuperframe(unsigned char* data);
            ~SacchSuperframe();
            unsigned int getMessageType();
            unsigned int getCallType();
            uint16_t getSourceUnitId();
            uint16_t getDestinationId();
        private:
            unsigned char* data;
    };

    class SacchSuperframeCollector {
        public:
            ~SacchSuperframeCollector();
            void push(Sacch* sacch);
            void reset();
            bool isComplete();
            SacchSuperframe* getSuperframe();
        private:
            Sacch* collected[4] = { nullptr };
    };

}