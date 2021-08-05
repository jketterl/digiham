#include "tact.hpp"

extern "C" {
#include "hamming_7_4.h"
}

using namespace Digiham::Dmr;

Tact* Tact::parse(unsigned char data) {
    if (hamming_7_4(&data)) return new Tact(data);
    return nullptr;
}

Tact::Tact(unsigned char data): data(data) {}

bool Tact::isBusy() {
    return (data >> 6) & 1;
}

unsigned char Tact::getSlot() {
    return (data >> 5) & 1;
}

unsigned char Tact::getLcss() {
    return (data >> 3) & 3;
}