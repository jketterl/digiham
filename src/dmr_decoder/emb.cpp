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