#pragma once

#include "lc.hpp"

namespace Digiham::Dmr {

    class EmbeddedCollector {
        public:
            EmbeddedCollector();
            ~EmbeddedCollector();
            void collect(unsigned char* data);
            void reset();
            Lc* getLc();
        private:
            unsigned char* data;
            unsigned char offset = 0;
    };

}