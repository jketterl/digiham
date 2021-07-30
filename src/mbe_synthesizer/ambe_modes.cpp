#include <cstring>
#include <sstream>
#include <iomanip>
#include <netinet/in.h>
#include "ambe_modes.hpp"

using namespace Digiham::Mbe;

TableMode::TableMode(const unsigned int index): index(index) {}

unsigned int TableMode::getIndex() const {
    return index;
}

ControlWordMode::ControlWordMode(short* cwds):
    cwds((short*) malloc(sizeof(short) * 6))
{
    std::memcpy((void *) this->cwds, cwds, sizeof(short) * 6);
}

ControlWordMode::~ControlWordMode() {
    free(cwds);
}

short* ControlWordMode::getCwds() {
    return cwds;
}

std::string ControlWordMode::getCwdsAsString() {
    std::stringstream ss;
    for (int i = 0; i < 6; i++) {
        if (i > 0) ss << ":";
        ss << std::setfill('0') << std::setw(4) << std::hex << htons(cwds[i]);
    }
    return ss.str();
}