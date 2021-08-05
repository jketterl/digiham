#include "emb.hpp"

extern "C" {
#include "quadratic_residue.h"
}

using namespace Digiham::Dmr;

Emb *Emb::parse(uint16_t data) {
    if (quadratic_residue(&data)) {
        return new Emb(data);
    }
    return nullptr;
}

Emb::Emb(uint16_t data): data(data) {}

unsigned char Emb::getColorCode() {
    return (data >> 12) & 0b1111;
}

unsigned char Emb::getLcss() {
    return (data >> 9) & 0b11;
}