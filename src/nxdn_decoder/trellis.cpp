#include "trellis.hpp"

using namespace Digiham::Nxdn;

unsigned int Trellis::decode(unsigned char* input, unsigned char* output, size_t len) {
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
