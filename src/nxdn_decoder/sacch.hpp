#pragma once

#include <cstdint>

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