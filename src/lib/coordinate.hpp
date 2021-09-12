#pragma once

namespace Digiham {

    class Coordinate {
        public:
            Coordinate(float lat, float lon);
            bool operator==(const Coordinate& other);

            float lat;
            float lon;
    };

}