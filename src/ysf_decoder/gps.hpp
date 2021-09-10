#pragma once

#include <stdint.h>
#include <stdbool.h>

namespace Digiham::Ysf {

    class Coordinate {
        public:
            static Coordinate* parse(const uint8_t* data);
            Coordinate(float lat, float lon);
            bool operator==(const Coordinate& other);

            float lat;
            float lon;
    };

}