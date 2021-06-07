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

        if (hamming_distance(potential_sync, (uint8_t*) header_sync, SYNC_SIZE) <= 2) {
            std::cerr << "found a header sync at pos " << read_pos << "\n";
            data->advance(read_pos, SYNC_SIZE);
            return new HeaderPhase();
        }

        if (hamming_distance(potential_sync, (uint8_t*) voice_sync, SYNC_SIZE) <= 1) {
            std::cerr << "found a voice sync at pos " << read_pos << "\n";
            data->advance(read_pos, SYNC_SIZE);
            return new VoicePhase(0);
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

    if (header->isVoice()) {
        delete(header);
        return new VoicePhase();
    }

    delete(header);
    return new SyncPhase();
}

VoicePhase::VoicePhase(int frameCount): Phase(), frameCount(frameCount) {
    scrambler = new Scrambler();
}

// default set up is with a sync due immediately, which is what we'd expect after a header
// when starting after a voice sync, pass frameCount = 0 to the constructor above
VoicePhase::VoicePhase(): VoicePhase(21) {}

VoicePhase::~VoicePhase() {
    delete scrambler;
}

Phase* VoicePhase::process(Ringbuffer* data, size_t& read_pos) {
    char* voice = (char*) malloc(sizeof(char) * 72);
    data->read(voice, read_pos, 72);
    data->advance(read_pos, 72);

    char* voice_packed = (char*) malloc(sizeof(char) * 9);
    memset(voice_packed, 0, 9);
    for (int i = 0; i < 72; i++) {
        voice_packed[i / 8] |= (voice[i] & 1) << ( i % 8 );
    }
    free(voice);

    fwrite(voice_packed, sizeof(char), 9, stdout);
    free(voice_packed);


    // only 24 bits of data, but the terminator is 48 bits long, so we have to look ahead
    uint8_t* data_frame = (uint8_t*) malloc(sizeof(uint8_t) * 48);
    data->read((char*) data_frame, read_pos, 48);
    data->advance(read_pos, 24);

    if (hamming_distance(data_frame, (uint8_t*) terminator, TERMINATOR_SIZE) == 0) {
        std::cerr << "terminator received\n";
        // move another 24 since it's clear that this is all used up now
        data->advance(read_pos, 24);
        return new SyncPhase();
    }
    if (isSyncDue()) {
        if (hamming_distance(data_frame, (uint8_t*) voice_sync, SYNC_SIZE) > 1) {
            if (syncMissing++ > 2) {
                std::cerr << "too many missed syncs, ending voice mode\n";
                return new SyncPhase();
            }
        }
        frameCount = 0;
    } else {
        frameCount++;

        scrambler->reset();
        char* data_descrambled = (char*) malloc(sizeof(char) * 24);
        scrambler->scramble((char*) data_frame, data_descrambled, 24);

        char* data_bytes = (char*) malloc(sizeof(char) * 3);
        memset(data_bytes, 0, sizeof(char) * 3);
        for (int i = 0; i < 24; i++) {
            data_bytes[i / 8] |= data_descrambled[i] << ( i % 8 );
        }
        free(data_descrambled);
        //DumpHex(data_bytes, 3);
        free(data_bytes);
    }

    free(data_frame);

    return this;
}

bool VoicePhase::isSyncDue() {
    return frameCount >= 20;
}