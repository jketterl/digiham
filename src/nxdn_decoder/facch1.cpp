#include "facch1.hpp"
#include "trellis.hpp"
#include <cstdlib>
#include <cstring>

using namespace Digiham::Nxdn;

Facch1* Facch1::parse(unsigned char* raw) {
    unsigned char* deinterleaved = (unsigned char*) malloc(sizeof(unsigned char) * 144);
    deinterleave(raw, deinterleaved);

    unsigned char* inflated = (unsigned char*) malloc(sizeof(unsigned char) * 24);
    inflate(deinterleaved, inflated);
    free(deinterleaved);

    unsigned char* trellis_decoded = (unsigned char*) malloc(sizeof(unsigned char) * 12);
    Trellis t;
    int rc = t.decode(inflated, trellis_decoded, 192);
    free(inflated);

    if (check_crc(trellis_decoded)) {
        return new Facch1(trellis_decoded);
    }

    free(trellis_decoded);
    return nullptr;
}

Facch1::Facch1(unsigned char* data): data(data) {}

Facch1::~Facch1() {
    free(data);
}

unsigned char Facch1::getMessageType() {
    return data[0] & 0b00111111;
}

void Facch1::deinterleave(unsigned char* in, unsigned char* out) {
    memset(out, 0, 72);
    for (int i = 0; i < 16; i++) {
        for (int k = 0; k < 9; k++) {
            int inpos = i * 9 + k;
            int outpos = k * 16 + i;
            out[outpos / 2] |= ((in[inpos / 2] >> (1 - inpos % 2)) & 1) << (1 - outpos % 2);
        }
    }
}

void Facch1::inflate(unsigned char* input, unsigned char* output) {
    memset(output, 0, 24);
    int pos = 0;
    for (int i = 0; i < 192; i++) {
        //std::cerr << "filling " << +i;
        bool x = 0;
        if ((i - 1) % 4 != 0) {
            //std::cerr << " from pos " << +pos << "\n";
            x = (input[pos / 2] >> (1 - pos % 2)) & 1;
            pos++;
        } else {
            //std::cerr << " with 0\n";
        }
        output[i / 8] |= x << (7 - i % 8);
    }
}

bool Facch1::check_crc(unsigned char* in) {
    // 12 registers, initial state
    uint16_t crc = 0b111111111111;
    for (int i = 0; i < 80; i++) {
        bool cb = ((crc >> 11) & 1) ^ ((in[i / 8] >> (7 - i % 8)) & 1);
        if (cb) {
            crc ^= 0b10000000111;
        }
        crc = ((crc << 1) & 0b111111111110) | cb;
    }

    uint16_t to_check = (((uint16_t) in[10]) << 4) | (in[11] >> 4);
    //std::cerr << "CRC result: " << +crc << "; should be: " << +to_check << "\n";
    return to_check == crc;
}