#pragma once

#include <cstdint>

#define LCSS_SINGLE 0
#define LCSS_START 1
#define LCSS_STOP 2
#define LCSS_CONTINUATION 3

namespace Digiham::Dmr {

    class Emb {
        public:
            static Emb* parse(uint16_t data);

            unsigned char getColorCode();
            unsigned char getLcss();
        private:
            explicit Emb(uint16_t data);
            uint16_t data;
    };

}