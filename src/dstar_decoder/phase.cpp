#include "phase.hpp"
#include <iostream>
#include <cstring>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::DStar;

Phase* SyncPhase::process(Ringbuffer* data, size_t& read_pos) {
    while (data->available(read_pos) > SYNC_SIZE) {
        //std::cerr << "scanning ringbuffer at " << read_pos << "\n";

        uint8_t potential_sync[SYNC_SIZE];
        data->read((char*) potential_sync, read_pos, SYNC_SIZE);

        if (hamming_distance(potential_sync, (uint8_t*) SyncPhase::header_sync, SYNC_SIZE) <= 2) {
            std::cerr << "found a sync at pos " << read_pos << "\n";
            data->advance(read_pos, SYNC_SIZE);
            return new HeaderPhase();
        }

        data->advance(read_pos, 1);
    }
    return nullptr;
}

Phase* HeaderPhase::process(Ringbuffer* data, size_t& read_pos) {
    char* raw = (char*) malloc(sizeof(char) * Header::bits);
    data->read(raw, read_pos, Header::bits);

    Header* header = Header::parse(raw);
    free(raw);
    if (header == nullptr) {
        data->advance(read_pos, 1);
        return new SyncPhase();
    }

    data->advance(read_pos, Header::bits);
    delete(header);
    // TODO: pass whatever phase comes next (voice / data)
    return new SyncPhase();
}