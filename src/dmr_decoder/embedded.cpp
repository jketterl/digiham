#include <cstdlib>
#include <cstring>
#include <cstdint>
#include "embedded.hpp"

extern "C" {
#include "hamming_16_11.h"
}

using namespace Digiham::Dmr;

EmbeddedCollector::EmbeddedCollector():
    data((unsigned char*) malloc(sizeof(unsigned char) * 16))
{}

EmbeddedCollector::~EmbeddedCollector() {
    free(data);
}

void EmbeddedCollector::collect(unsigned char *toCollect) {
    if (offset > 3) {
        return;
    }
    std::memcpy(data + (offset * 4), toCollect, 4);
    offset++;
}

void EmbeddedCollector::reset() {
    offset = 0;
}

Lc *EmbeddedCollector::getLc() {
    if (offset < 3) {
        return nullptr;
    }

    uint16_t decode_matrix[8];
    for (int i = 0; i < 16; i++) {
        uint8_t byte = data[i];
        for (int k = 0; k < 8; k++) {
            decode_matrix[k] = (decode_matrix[k] << 1) | ((byte >> (7 - k)) & 1);
        }
    }

    for (int i = 0; i < 7; i++) {
        if (!hamming_16_11(&decode_matrix[i])) {
            // incorrectable error in FEC :(
            return nullptr;
        }
    }

    // column parity check as described in B.2.1
    uint16_t parity = 0;
    for (int i = 0; i < 8; i++) {
        parity ^= decode_matrix[i];
    }
    if (parity != 0) {
        // parity error
        return nullptr;
    }

    // de-interleave according to etsi B.2.1
    auto lc = (unsigned char*) malloc(9);
    lc[0] =  (decode_matrix[0] & 0b1111111100000000) >> 8;
    lc[1] =  (decode_matrix[0] & 0b0000000011100000)       | ((decode_matrix[1] & 0b1111100000000000) >> 11);
    lc[2] = ((decode_matrix[1] & 0b0000011111100000) >> 3) | ((decode_matrix[2] & 0b1100000000000000) >> 14);
    lc[3] =  (decode_matrix[2] & 0b0011111111000000) >> 6;
    lc[4] =  (decode_matrix[3] & 0b1111111100000000) >> 8;
    lc[5] =  (decode_matrix[3] & 0b0000000011000000)       | ((decode_matrix[4] & 0b1111110000000000) >> 10);
    lc[6] = ((decode_matrix[4] & 0b0000001111000000) >> 2) | ((decode_matrix[5] & 0b1111000000000000) >> 12);
    lc[7] = ((decode_matrix[5] & 0b0000111111000000) >> 4) | ((decode_matrix[6] & 0b1100000000000000) >> 14);
    lc[8] =  (decode_matrix[6] & 0b0011111111000000) >> 6;

    // checksum calculation according to etsi B.2.1 and B.3.11
    uint16_t checksum = 0;
    for (int i = 0; i < 9; i++) {
        checksum += lc[i];
    }
    uint8_t checksum_mod = checksum % 31;

    uint8_t received_checksum = 0;
    for (int i = 0; i < 5; i++) {
        received_checksum |= (decode_matrix[i+2] & 0b0000000000100000) >> (i + 1);
    }

    if (checksum_mod != received_checksum) {
        // checksum error
        return nullptr;
    }

    Lc* result = new Lc(lc);
    free(lc);
    return result;
}