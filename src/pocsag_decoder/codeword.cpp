#include "codeword.hpp"

extern "C" {
#include "bch_31_21.h"
}

using namespace Digiham::Pocsag;

Codeword* Codeword::parse(unsigned char* input) {
    uint32_t codeword = 0;
    for (int i = 0; i < CODEWORD_SIZE; i++) {
        codeword |= (input[i] && 0b1) << (31 - i);
    }

    uint32_t codeword_payload = codeword >> 1;
    if (!bch_31_21(&codeword_payload)) {
        return nullptr;
    }
    codeword = (codeword & 0b1) | (codeword_payload << 1);

    bool parity = 0;
    for (int i = 0; i < CODEWORD_SIZE; i++) {
        parity ^= (codeword >> i) & 0b1;
    }

    if (parity) {
        return nullptr;
    }

    return new Codeword(codeword);
}

Codeword::Codeword(uint32_t data): data(data) {}

bool Codeword::isIdle() const {
    return data == idle_codeword;
}

uint32_t Codeword::getPayload() const {
    // 20 bits
    return (data >> 11) & ((1 << 20) - 1);
}

bool Codeword::isAddressCodeword() const {
    return (data >> 31) == 0;
}

uint32_t Codeword::getAddress() const {
    // 18 bits
    return (data >> 13) & ((1 << 18) - 1);
}

unsigned char Codeword::getFunctionBits() const {
    return (data >> 11) & 0b11;
}