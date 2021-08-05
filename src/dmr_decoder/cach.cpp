#include <cstdlib>

#include "cach.hpp"

using namespace Digiham::Dmr;

Cach* Cach::parse(const unsigned char *raw) {
    unsigned char tact = 0;
    for (int i = 0; i < 7; i++) {
        unsigned char bit = tact_positions[i];
        int pos = bit / 2;
        int shift = 1 - (bit % 2);

        tact = (tact << 1) | ((raw[pos] >> shift) & 1);
    }

    auto cach_payload = (unsigned char*) calloc(sizeof(unsigned char), 3);
    for (int i = 0; i < 17; i++) {
        unsigned char bit = payload_positions[i];
        int pos = bit / 2;
        int shift = 1 - (bit % 2);

        cach_payload[i / 8] |= ((raw[pos] >> shift) & 1) << (i % 8);
    }

    return new Cach(Tact::parse(tact), cach_payload);
}

Cach::Cach(Tact* tact, unsigned char* payload): tact(tact), payload(payload) {}

Cach::~Cach() {
    free(payload);
    delete tact;
}

bool Cach::hasTact() {
    return tact != nullptr;
}

Tact* Cach::getTact() {
    return tact;
}