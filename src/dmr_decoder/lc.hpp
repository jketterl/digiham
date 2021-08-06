#pragma once

#include <cstdint>

#define LC_OPCODE_GROUP 0
#define LC_OPCODE_UNIT_TO_UNIT 3

#define LC_TALKER_ALIAS_HDR 4
#define LC_TALKER_ALIAS_BLK1 5
#define LC_TALKER_ALIAS_BLK2 6
#define LC_TALKER_ALIAS_BLK3 7
#define LC_GPS_INFO 8

namespace Digiham::Dmr {

    class Lc {
        public:
            static Lc* parseFromVoiceHeader(unsigned char* data);
            explicit Lc(unsigned char* data);
            ~Lc();
            unsigned char getOpCode();
            unsigned char getFeatureSetId();
            uint32_t getSource();
            uint32_t getTarget();
            unsigned char* getData();
        private:
            unsigned char* data;
    };

}