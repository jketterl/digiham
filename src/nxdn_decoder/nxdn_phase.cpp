#include "nxdn_phase.hpp"

#include <iostream>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::Nxdn;

// -3. +1, -3, +3, -3, -3, +3, +3, -1, +3
const uint8_t SyncPhase::frameSync[SYNC_SIZE] = { 3, 0, 3, 1, 3, 3, 1, 1, 2, 1 };

Digiham::Phase* SyncPhase::process(Ringbuffer* data, size_t& read_pos) {
    //std::cerr << "scanning ringbuffer at " << read_pos << "\n";

    uint8_t potential_sync[SYNC_SIZE];
    data->read((char*) potential_sync, read_pos, SYNC_SIZE);

    if (hamming_distance(potential_sync, (uint8_t*) frameSync, SYNC_SIZE) <= 2) {
        std::cerr << "found a frame sinc at pos " << read_pos << "\n";
        data->advance(read_pos, SYNC_SIZE);
        // return new HeaderPhase();
    }

    // as long as we don't find any sync, move ahead, bit by bit
    data->advance(read_pos, 1);
    // tell decoder that we'll continue
    return this;
}
