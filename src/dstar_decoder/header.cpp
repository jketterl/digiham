#include "header.hpp"
#include <cstdlib>
#include <cstring>

#include <iostream>

extern "C" {
#include "dumphex.h"
#include "hamming_distance.h"
}

using namespace Digiham::DStar;

Header::Header(char* data) {
    this->data = data;
}

Header::~Header() {
    delete data;
}

Header* Header::parse(char* raw) {
    char* descrambled = (char*) malloc(sizeof(char) * Header::bits);
    descramble(raw, descrambled);

    char* deinterleaved = (char*) malloc(sizeof(char) * Header::bits);
    deinterleave(descrambled, deinterleaved);
    free(descrambled);

    char* decoded = (char*) malloc(sizeof(char) * ((Header::bits / 2 + 7) / 8));
    unsigned int errors = viterbi_decode(deinterleaved, decoded);
    std::cerr << "header viterbi errors: " << errors << "\n";
    free(deinterleaved);

    // TODO: header FEC in P_FCS

    if (errors < 10) {
        DumpHex(decoded, 41);
        return new Header(decoded);
    }

    free(decoded);
    return nullptr;
}

void Header::deinterleave(char* in, char* out) {
    memset(out, 0, (Header::bits + 7) / 8);

    for (int i = 0; i < 12; i++){
        for (int k = 0; k < 28; k++) {
            out[k * 24 + i] = in[i * 28 + k];
        }
    }

    for (int i = 12; i < 24; i++) {
        for (int k = 0; k < 27; k++) {
            out[ k * 24 + i] = in[12 + i * 27 + k];
        }
    }
}

typedef struct {
    uint16_t metric;
    uint8_t *data;
} branch;

const uint8_t Header::trellis_transitions[4][2] = {
    {0b00, 0b11}, // 00
    {0b11, 0b00}, // 01
    {0b10, 0b01}, // 10
    {0b01, 0b10}, // 11
};

unsigned int Header::viterbi_decode(char* input, char* output) {
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
                uint8_t transition = Header::trellis_transitions[previous_state][outbit];
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

void Header::descramble(char* input, char* output) {
    // init with 1s according to spec
    uint8_t wsr = 0b1111111;
    memset(output, 0, 660);
    for (int i = 0; i < 660; i++) {
        // calculate whitening bit
        bool wb = (wsr & 1) ^ ((wsr >> 3) & 1);

        // apply
        output[i] = (input[i] & 1) ^ wb;

        // rotate
        wsr = ((wsr & 0b1111110) >> 1) | (wb << 6);
    }
}

bool Header::isData() {
    return (data[0] >> 7) & 1;
}