#pragma once

#include <cstdint>

#include "coordinate.hpp"

namespace Digiham::Ysf {

    class Gps {
        public:
            static Coordinate* parse(const uint8_t* data);
    };

}