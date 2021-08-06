#pragma once

#include <cstdint>

// data types according to ETSI 9.3.6
#define DATA_TYPE_PI 0
#define DATA_TYPE_VOICE_LC 1
#define DATA_TYPE_TERMINATOR_LC 2
#define DATA_TYPE_CSBK 3
#define DATA_TYPE_MBC 4
#define DATA_TYPE_MBC_CONTINUATION 5
#define DATA_TYPE_DATA_HEADER 6
#define DATA_TYPE_RATE_1_2_DATA 7
#define DATA_TYPE_RATE_3_4_DATA 8
#define DATA_TYPE_IDLE 9
#define DATA_TYPE_RATE_1_DATA 10
#define DATA_TYPE_UNIFIED_SINGLE_BLOCK_DATA 11
// the remainder is reserved, leaving them out for now

namespace Digiham::Dmr {

    class SlotType {
        public:
            static SlotType* parse(uint32_t raw);
            unsigned char getColorCode();
            unsigned char getDataType();
        private:
            explicit SlotType(uint32_t data);
            uint32_t data;
    };

}