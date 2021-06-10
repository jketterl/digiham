#include "nxdn_phase.hpp"
#include "sacch.hpp"

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

FramedPhase::FramedPhase() {
    scrambler = new Scrambler();
}

FramedPhase::~FramedPhase() {
    delete scrambler;
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

    scrambler->reset();

    unsigned char lich_raw[8];
    data->read((char*) lich_raw, read_pos, 8);
    data->advance(read_pos, 8);

    unsigned char lich_descrambled[8];
    scrambler->scramble(lich_raw, lich_descrambled, 8);

    Lich* new_lich = Lich::parse(lich_descrambled);
    if (new_lich != nullptr) {
        if (lich != nullptr) delete lich;
        lich = new_lich;
    }
    if (lich != nullptr) {
        if (
            lich->getRFType() != NXDN_RF_CHANNEL_TYPE_RCCH &&
            lich->getFunctionalType() != NXDN_USC_TYPE_UDCH
        ) {
            // looks like we're in voice mode
            unsigned char sacch_raw[30];
            data->read((char*) sacch_raw, read_pos,  30);

            unsigned char sacch_descrambled[30];
            scrambler->scramble(sacch_raw, sacch_descrambled, 30);

            Sacch* sacch = Sacch::parse(sacch_descrambled);
            if (sacch != nullptr) {
                delete sacch;
            }
            data->advance(read_pos, 30);

            unsigned char option = lich->getOption();

            // 4 voice frames
            for (int i = 0; i < 2; i++) {
                // always read and scramble now until FACCH is implemented
                unsigned char voice[72];
                data->read((char*) voice, read_pos, 72);

                unsigned char voice_descrambled[72];
                scrambler->scramble(voice, voice_descrambled, 72);

                // evaluate steal flag
                if ((option >> (1 - i)) & 1) {
                    unsigned char voice_frame[18] = { 0 };
                    for (int k = 0; k < 72; k++) {
                        voice_frame[k / 4] |= (voice_descrambled[k] & 3) << (6 - ((k % 4) * 2));
                    }

                    fwrite(voice_frame, sizeof(char), 18, stdout);
                    fflush(stdout);
                } else {
                    // std::cerr << "steal! ";
                    // TODO: parse FAACH1
                }
                data->advance(read_pos, 72);
            }

        } else {
            // advance what's left
            data->advance(read_pos, 174);
        }
    } else {
        std::cerr << "could not parse lich\n";
        // advance what's left
        data->advance(read_pos, 174);
    }

    return nullptr;
}