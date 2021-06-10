#include "sacch.hpp"
#include <cstdlib>
#include <cstdint>
#include <cstring>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::Nxdn;

Sacch* Sacch::parse(unsigned char* data) {
    unsigned char* deinterleaved = (unsigned char*) malloc(sizeof(char) * 30);
    deinterleave(data, deinterleaved);

    free(deinterleaved);
    return nullptr;
}

void Sacch::deinterleave(unsigned char* in, unsigned char* out) {
    for (int i = 0; i < 5; i++) {
        for (int k = 0; k < 6; k++) {
            out[k * 5 + i] = in[i * 6 + k];
        }
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
    uint8_t shift = 0;

    uint8_t i;
    branch *branches = (branch*) malloc(sizeof(branch) * 4);
    for (i = 0; i < 4; i++) {
        branches[i].metric = 0;
        branches[i].data = (uint8_t*) malloc(42);
        memset(branches[i].data, 0, 42);
    }

    for (int pos = 0; pos < 330; pos++) {
        uint8_t in_transition = ((input[pos * 2] & 1) << 1) | (input[pos * 2 + 1] & 1);

        uint8_t outpos = pos / 8;
        // reverse bit order on the fly
        uint8_t outshift = pos % 8;
        // this would be straight
        // uint8_t outshift = 7 - pos % 8

        branch *next_branches = (branch*) malloc(sizeof(branch) * 4);

        for (i = 0; i < 4; i++) {
            uint8_t k;
            uint16_t best_metric = -1;
            uint8_t selected = -1;
            uint8_t outbit = (i & 0b10) >> 1;
            for (k = 0; k < 2; k++) {
                uint8_t previous_state = (( i << 1 ) & 0b10 ) | k;
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
            selected_branch.data = (uint8_t*) malloc(42);
            memcpy(selected_branch.data, branches[selected].data, 42);
            selected_branch.data[outpos] |= ( outbit << outshift );
            next_branches[i] = selected_branch;

        }

        for (i = 0; i < 4; i++) free(branches[i].data);
        free(branches);
        branches = next_branches;
    }

    branch best_branch;
    for (i = 0; i < 4; i++) {
        branch candidate = branches[i];
        if (i == 0 || candidate.metric < best_branch.metric) best_branch = candidate;
    }

    memcpy(output, best_branch.data, 42);
    uint16_t best_metric = best_branch.metric;

    for (i = 0; i < 4; i++) free(branches[i].data);
    free(branches);

    return best_metric;
}
