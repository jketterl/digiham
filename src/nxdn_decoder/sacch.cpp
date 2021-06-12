#include "sacch.hpp"
#include <cstdlib>
#include <cstring>

extern "C" {
#include "hamming_distance.h"
}

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
    int rc = viterbi_decode(inflated, trellis_decoded);
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
        //std::cerr << "filling " << +i;
        bool x = 0;
        if ((i + 1) % 6 != 0) {
            //std::cerr << " from pos " << +pos << "\n";
            x = (input[pos / 2] >> (1 - pos % 2)) & 1;
            pos++;
        } else {
            //std::cerr << " with 0\n";
        }
        output[i / 8] |= x << (7 - i % 8);
    }
}

typedef struct {
    uint16_t metric;
    uint8_t *data;
} branch;

const uint8_t Sacch::trellis_transitions[16][2] = {
    {0b00, 0b11}, // 0000
    {0b11, 0b00}, // 0001
    {0b10, 0b01}, // 0010
    {0b01, 0b10}, // 0011
    {0b01, 0b10}, // 0100
    {0b10, 0b01}, // 0101
    {0b11, 0b00}, // 0110
    {0b00, 0b11}, // 0111
    {0b01, 0b10}, // 1000
    {0b10, 0b01}, // 1001
    {0b11, 0b00}, // 1010
    {0b00, 0b11}, // 1011
    {0b00, 0b11}, // 1100
    {0b11, 0b00}, // 1101
    {0b10, 0b01}, // 1110
    {0b01, 0b10}, // 1111
};

unsigned int Sacch::viterbi_decode(unsigned char* input, unsigned char* output) {
    uint8_t i;
    branch *branches = (branch*) malloc(sizeof(branch) * 16);
    for (i = 0; i < 16; i++) {
        branches[i].metric = 0;
        branches[i].data = (uint8_t*) malloc(5);
        memset(branches[i].data, 0, 5);
    }

    for (int pos = 0; pos < 36; pos++) {
        uint8_t in_transition = (input[pos / 4] >> (2 * (3 - pos % 4))) & 0b11;

        uint8_t outpos = pos / 8;
        uint8_t outshift = 7 - pos % 8;

        branch *next_branches = (branch*) malloc(sizeof(branch) * 16);

        for (i = 0; i < 16; i++) {
            uint8_t k;
            uint16_t best_metric = -1;
            uint8_t selected = -1;
            uint8_t outbit = (i & 0b1000) >> 3;
            for (k = 0; k < 2; k++) {
                uint8_t previous_state = (( i << 1 ) & 0b1110 ) | k;
                uint8_t transition = Sacch::trellis_transitions[previous_state][outbit];
                branch to_evaluate = branches[previous_state];
                uint16_t metric = to_evaluate.metric + hamming_distance(&in_transition, &transition, 1);

                if (k == 0 || metric < best_metric) {
                    best_metric = metric;
                    selected = previous_state;
                }
            }

            branch selected_branch;
            selected_branch.metric = best_metric;
            selected_branch.data = (uint8_t*) malloc(5);
            memcpy(selected_branch.data, branches[selected].data, 5);
            selected_branch.data[outpos] |= ( outbit << outshift );
            next_branches[i] = selected_branch;

        }

        for (i = 0; i < 16; i++) free(branches[i].data);
        free(branches);
        branches = next_branches;
    }

    branch best_branch;
    for (i = 0; i < 16; i++) {
        branch candidate = branches[i];
        if (i == 0 || candidate.metric < best_branch.metric) best_branch = candidate;
    }

    memcpy(output, best_branch.data, 5);
    uint16_t best_metric = best_branch.metric;

    for (i = 0; i < 16; i++) free(branches[i].data);
    free(branches);

    return best_metric;
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
    //std::cerr << "CRC result: " << +crc << "; should be: " << +to_check << "\n";
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