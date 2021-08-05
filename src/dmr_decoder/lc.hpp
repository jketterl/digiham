#pragma once

#include <cstdint>

#define LC_OPCODE_GROUP 0
#define LC_OPCODE_UNIT_TO_UNIT 3

namespace Digiham::Dmr {

    class Lc {
        public:
            explicit Lc(unsigned char* data);
            ~Lc();
            unsigned char getOpCode();
            uint32_t getSource();
            uint32_t getTarget();
        private:
            unsigned char* data;
    };

}