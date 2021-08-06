#include "lc.hpp"

#include <malloc.h>
#include <cstring>

using namespace Digiham::Dmr;

Lc* Lc::parseFromVoiceHeader(unsigned char *data) {
    // TODO implement Reed-Solomon (12, 9) FEC according to B.3.6
    return new Lc(data);
}

Lc::Lc(unsigned char* data):
    data((unsigned char*) malloc(sizeof(unsigned char) * 9))
{
    std::memcpy(this->data, data, 9);
}

Lc::~Lc() {
    free(data);
}

unsigned char Lc::getOpCode() {
    return data[0] & 0b00111111;
}

unsigned char Lc::getFeatureSetId() {
    return data[1];
}

uint32_t Lc::getSource() {
    uint32_t source_id = data[6] << 16 | data[7] << 8 | data[8];
    return source_id;
}

uint32_t Lc::getTarget() {
    uint32_t target_id = data[3] << 16 | data[4] << 8 | data[5];
    return target_id;
}

unsigned char* Lc::getData() {
    return data + 2;
}