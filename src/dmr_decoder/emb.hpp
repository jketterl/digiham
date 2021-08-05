#pragma once

#include <cstdint>

namespace Digiham::Dmr {

    class Emb {
        public:
            static Emb* parse(uint16_t data);
        private:
            explicit Emb(uint16_t data);
            uint16_t data;
    };

}