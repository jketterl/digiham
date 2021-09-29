#include "sacch.hpp"
#include "trellis.hpp"
#include <cstdlib>
#include <cstring>

using namespace Digiham::Nxdn;

Sacch::Sacch(unsigned char* data) {
    this->data = data;
}

Sacch::~Sacch() {
    free(data);
}

unsigned char Sacch::getStructureIndex() {
    return (data[0] >> 6) ^ 0b11;
}

unsigned char* Sacch::getSuperframeData() {
    return data + 1;
}

Sacch* Sacch::parse(unsigned char* data) {
    unsigned char* deinterleaved = (unsigned char*) malloc(sizeof(unsigned char) * 30);
    deinterleave(data, deinterleaved);

    unsigned char* inflated = (unsigned char*) malloc(sizeof(unsigned char) * 9);
    inflate(deinterleaved, inflated);
    free(deinterleaved);

    unsigned char* trellis_decoded = (unsigned char*) malloc(sizeof(unsigned char) * 5);
    Trellis t;
    int rc = t.decode(inflated, trellis_decoded, 72);
    free(inflated);

    if (check_crc(trellis_decoded)) {
        return new Sacch(trellis_decoded);
    }

    free(trellis_decoded);
    return nullptr;
}

void Sacch::deinterleave(unsigned char* in, unsigned char* out) {
    memset(out, 0, 30);
    for (int i = 0; i < 12; i++) {
        for (int k = 0; k < 5; k++) {
            int inpos = i * 5 + k;
            int outpos = k * 12 + i;
            out[outpos / 2] |= ((in[inpos / 2] >> (1 - inpos % 2)) & 1) << (1 - outpos % 2);
        }
    }
}

void Sacch::inflate(unsigned char* input, unsigned char* output) {
    memset(output, 0, 9);
    int pos = 0;
    for (int i = 0; i < 72; i++) {
        bool x = 0;
        if ((i + 1) % 6 != 0) {
            x = (input[pos / 2] >> (1 - pos % 2)) & 1;
            pos++;
        }
        output[i / 8] |= x << (7 - i % 8);
    }
}

bool Sacch::check_crc(unsigned char* in) {
    // 6 registers, initial state
    uint8_t crc = 0b00111111;
    for (int i = 0; i < 26; i++) {
        bool cb = ((crc >> 5) & 1) ^ ((in[i / 8] >> (7 - i % 8)) & 1);
        if (cb) {
            //crc ^= 0b00110010;
            crc ^= 0b00010011;
        }
        crc = ((crc << 1) & 0b00111110) | cb;
    }

    uint8_t to_check = in[3] & 0b00111111;
    return to_check == crc;
}

SacchSuperframeCollector::~SacchSuperframeCollector() {
    reset();
}

void SacchSuperframeCollector::push(Sacch* sacch) {
    int index = sacch->getStructureIndex();
    if (index > 0 && collected[index - 1] == nullptr) {
        // good sacch, but we don't have the fragment before it, so it's worthless
        delete sacch;
        return;
    }
    collected[sacch->getStructureIndex()] = sacch;
}

void SacchSuperframeCollector::reset() {
    for (int i = 0; i < 4; i++) {
        if (collected[i] != nullptr) {
            delete(collected[i]);
            collected[i] = nullptr;
        }
    }
}

bool SacchSuperframeCollector::isComplete() {
    for (int i = 0; i < 4; i++) {
        if (collected[i] == nullptr) {
            return false;
        }
    }
    return true;
}

SacchSuperframe* SacchSuperframeCollector::getSuperframe() {
    if (!isComplete()) return nullptr;
    unsigned char* data = (unsigned char*) malloc(sizeof(unsigned char) * 9);
    memset(data, 0, 9);

    for (int i = 0; i < 4; i++) {
        unsigned char* sfdata = collected[i]->getSuperframeData();
        for (int k = 0; k < 18; k++) {
            int outpos = i * 18 + k;
            data[outpos / 8] |= ((sfdata[k / 8] >> (7 - k % 8)) & 1) << (7 - outpos % 8);
        }
    }

    return new SacchSuperframe(data);
}

SacchSuperframe::SacchSuperframe(unsigned char* data) {
    this->data = data;
}

SacchSuperframe::~SacchSuperframe() {
    free(data);
}

unsigned int SacchSuperframe::getMessageType() {
    return data[0] & 0b00111111;
}

unsigned int SacchSuperframe::getCallType() {
    return data[2] >> 5;
}

uint16_t SacchSuperframe::getSourceUnitId() {
    return (((uint16_t) data[3]) << 8) | data[4];
}

uint16_t SacchSuperframe::getDestinationId() {
    return (((uint16_t) data[5]) << 8) | data[6];
}