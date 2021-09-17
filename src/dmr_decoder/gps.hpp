#pragma once

#include "coordinate.hpp"

namespace Digiham::Dmr {

    class Gps {
        public:
            static Digiham::Coordinate* parse(const unsigned char* data);
    };

}