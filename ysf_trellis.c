#include "ysf_trellis.h"
#include "ysf_bitmappings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t hamming_distance(uint8_t a, uint8_t b) {
    uint8_t xored = a ^ b;
    uint8_t i;
    uint8_t result = 0;
    for (i = 0; i < 8; i++) {
        result += (xored >> i) & 1;
    }
    return result;
}

typedef struct {
    uint8_t metric;
    uint8_t *data;
} branch;

void decode_trellis(uint8_t *input, uint8_t size, uint8_t *output) {
    uint8_t pos = 0;
    uint8_t shift = 0;

    uint8_t i;
    branch *branches = (branch*) malloc(sizeof(branch) * 16);
    for (i = 0; i < 16; i++) {
        branches[i].metric = 0;
        branches[i].data = (uint8_t*) malloc(100);
        memset(branches[i].data, 0, 100);
    }

    while (pos < size) {
        uint8_t inpos = pos / 4;
        uint8_t inshift = 2 * ( 3 - pos % 4 );
        uint8_t in_transition = (input[inpos] >> inshift) & 0b11;

        uint8_t outpos = pos / 8;
        uint8_t outshift = ( 7 - pos % 8 );

        //fprintf(stderr, "input: %i\n", in_transition);

        branch *next_branches = (branch*) malloc(sizeof(branch) * 16);
        
        for (i = 0; i < 16; i++) {
            //fprintf(stderr, "evaluating state %i", i);

            uint8_t k;
            uint8_t best_metric = -1;
            uint8_t selected = -1;
            uint8_t outbit = (i & 0b1000) >> 3;
            //fprintf(stderr, " outbit: %i", outbit);
            for (k = 0; k < 2; k++) {
                uint8_t previous_state = (( i << 1 ) & 0b1110 ) | k;
                uint8_t transition = trellis_transitions[previous_state][outbit];
                branch to_evaluate = branches[previous_state];
                uint8_t metric = to_evaluate.metric + hamming_distance(in_transition, transition);
                //fprintf(stderr, " metric for previous %i: %i", previous_state, metric);

                if (k == 0 || metric < best_metric) {
                    best_metric = metric;
                    selected = previous_state;
                }
            }

            branch selected_branch;
            selected_branch.metric = best_metric;
            selected_branch.data = (uint8_t*) malloc(100);
            memcpy(selected_branch.data, branches[selected].data, 100);
            selected_branch.data[outpos] = selected_branch.data[outpos] | ( outbit << outshift );
            next_branches[i] = selected_branch;
            //fprintf(stderr, " selected: %i\n", selected);

        }

        for (i = 0; i < 16; i++) free(branches[i].data);
        free(branches);
        branches = next_branches;

        pos++;
    }

    branch best_branch;
    for (i = 0; i < 16; i++) {
        branch candidate = branches[i];
        if (i == 0 || candidate.metric < best_branch.metric) best_branch = candidate;
    }

    //fprintf(stderr, "metric for best branch: %i\n", best_branch.metric);
    //fprintf(stderr, "data: %i %i\n", best_branch.data[0], best_branch.data[1]);
    memcpy(output, best_branch.data, 100);

    free(branches);
}
