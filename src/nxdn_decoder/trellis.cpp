#include "trellis.hpp"
#include <cstring>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::Nxdn;

const uint8_t Trellis::transitions[16][2] = {
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

unsigned int Trellis::decode(unsigned char* input, unsigned char* output, size_t len) {
    uint8_t i;
    branch *branches = (branch*) malloc(sizeof(branch) * 16);
    uint8_t data_size = (len + 7) / 8;
    for (i = 0; i < 16; i++) {
        branches[i].metric = 0;
        branches[i].data = (uint8_t*) malloc(data_size);
        memset(branches[i].data, 0, data_size);
    }

    for (int pos = 0; pos < len / 2; pos++) {
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
                uint8_t transition = Trellis::transitions[previous_state][outbit];
                branch to_evaluate = branches[previous_state];
                uint16_t metric = to_evaluate.metric + hamming_distance(&in_transition, &transition, 1);

                if (k == 0 || metric < best_metric) {
                    best_metric = metric;
                    selected = previous_state;
                }
            }

            branch selected_branch;
            selected_branch.metric = best_metric;
            selected_branch.data = (uint8_t*) malloc(data_size);
            memcpy(selected_branch.data, branches[selected].data, data_size);
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

    memcpy(output, best_branch.data, data_size);
    uint16_t best_metric = best_branch.metric;

    for (i = 0; i < 16; i++) free(branches[i].data);
    free(branches);

    return best_metric;
}
