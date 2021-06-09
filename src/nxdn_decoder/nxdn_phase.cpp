#include "nxdn_phase.hpp"
#include "lich.hpp"

#include <iostream>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::Nxdn;

// -3. +1, -3, +3, -3, -3, +3, +3, -1, +3
const uint8_t Phase::frameSync[SYNC_SIZE] = { 3, 0, 3, 1, 3, 3, 1, 1, 2, 1 };

Digiham::Phase* SyncPhase::process(Ringbuffer* data, size_t& read_pos) {
    //std::cerr << "scanning ringbuffer at " << read_pos << "\n";

    uint8_t potential_sync[SYNC_SIZE];
    data->read((char*) potential_sync, read_pos, SYNC_SIZE);

    if (hamming_distance(potential_sync, (uint8_t*) frameSync, SYNC_SIZE) <= 2) {
        std::cerr << "found a frame sync at pos " << read_pos << "\n";
        //data->advance(read_pos, SYNC_SIZE);
        return new FramedPhase();
    }

    // as long as we don't find any sync, move ahead, bit by bit
    data->advance(read_pos, 1);
    // tell decoder that we'll continue
    return this;
}

Digiham::Phase* FramedPhase::process(Ringbuffer* data, size_t& read_pos) {
    unsigned char sync[SYNC_SIZE];
    data->read((char*) sync, read_pos, SYNC_SIZE);
    if (hamming_distance(sync, (uint8_t*) frameSync, SYNC_SIZE) <= 2) {
        // increase certainty, cap at 3
        if (++syncCount > 3) syncCount = 3;
    } else {
        if (--syncCount < 0) {
            std::cerr << "lost sync at " << read_pos << "\n";
            return new SyncPhase();
        }
    }
    data->advance(read_pos, SYNC_SIZE);

    unsigned char lich_raw[8];
    data->read((char*) lich_raw, read_pos, 8);

    Lich* lich = Lich::parse(lich_raw);
    if (lich != nullptr) delete(lich);

    return nullptr;
}