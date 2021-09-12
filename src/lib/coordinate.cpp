#include "coordinate.hpp"

using namespace Digiham;

Coordinate::Coordinate(float lat, float lon): lat(lat), lon(lon) {}

bool Coordinate::operator==(const Coordinate &other) {
    return other.lat == lat && other.lon == lon;
}
