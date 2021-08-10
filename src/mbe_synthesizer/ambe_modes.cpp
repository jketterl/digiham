#include <cstring>
#include <sstream>
#include <iomanip>
#include <utility>
#include <netinet/in.h>
#include "ambe_modes.hpp"

using namespace Digiham::Mbe;

TableMode::TableMode(const unsigned int index): index(index) {}

unsigned int TableMode::getIndex() const {
    return index;
}

bool TableMode::operator==(const Mode &other) {
    try {
        auto tmode = dynamic_cast<const TableMode&>(other);
        return tmode.getIndex() == getIndex();
    } catch (std::bad_cast&) {
        return false;
    }
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

bool ControlWordMode::operator==(const Mode &other) {
    try {
        auto cmode = dynamic_cast<const ControlWordMode&>(other);
        return std::memcmp(cmode.getCwds(), getCwds(), sizeof(short) * 6) == 0;
    } catch (std::bad_cast&) {
        return false;
    }
}

DynamicMode::DynamicMode(std::function<Mode* (unsigned char)> callback):
    callback(std::move(callback))
{}

Mode *DynamicMode::getModeFor(unsigned char code) {
    return callback(code);
}

bool DynamicMode::operator==(const Mode &other) {
    return &other == this;
}