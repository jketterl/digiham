#include "nxdn_phase.hpp"
#include "facch1.hpp"
#include "types.hpp"

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
    sacchCollector = new SacchSuperframeCollector();
}

FramedPhase::~FramedPhase() {
    delete scrambler;
    delete sacchCollector;
}

Digiham::Phase* FramedPhase::process(Ringbuffer* data, size_t& read_pos) {
    unsigned char sync[SYNC_SIZE];
    data->read((char*) sync, read_pos, SYNC_SIZE);
    if (hamming_distance(sync, (uint8_t*) frameSync, SYNC_SIZE) <= 2) {
        // increase certainty, cap at 6
        if (++syncCount > 6) syncCount = 6;
    } else {
        if (--syncCount < 0) {
            std::cerr << "lost sync at " << read_pos << "\n";
            ((MetaWriter*) meta)->reset();
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
    if (
        lich != nullptr &&
        lich->getRFType() != NXDN_RF_CHANNEL_TYPE_RCCH &&
        lich->getFunctionalType() != NXDN_USC_TYPE_UDCH
    ) {
        /*
        // some raw data for testing, taken from the "Common Air interface Test" document
        // when decoded, these should for one superframe SACCH with some sample VOICECALL information.
        unsigned char sacch_raw[30] = {
            0b11, 0b00, 0b11, 0b11, 0b10, 0b10, 0b00, 0b00,
            0b10, 0b10, 0b01, 0b10, 0b11, 0b10, 0b10, 0b00,
            0b10, 0b10, 0b00, 0b10, 0b00, 0b11, 0b01, 0b01,
            0b01, 0b10, 0b11, 0b10, 0b10, 0b00
        };
        unsigned char sacch_raw[30] = {
            0b11, 0b00, 0b01, 0b10, 0b11, 0b01, 0b10, 0b11,
            0b10, 0b11, 0b00, 0b00, 0b11, 0b10, 0b10, 0b11,
            0b00, 0b11, 0b10, 0b10, 0b01, 0b00, 0b00, 0b10,
            0b01, 0b10, 0b10, 0b10, 0b10, 0b00
        };
        unsigned char sacch_raw[30] = {
            0b01, 0b10, 0b00, 0b11, 0b10, 0b10, 0b00, 0b01,
            0b10, 0b11, 0b01, 0b00, 0b10, 0b10, 0b10, 0b00,
            0b00, 0b01, 0b10, 0b10, 0b10, 0b00, 0b11, 0b10,
            0b00, 0b10, 0b10, 0b00, 0b00, 0b00
        };
        unsigned char sacch_raw[30] = {
            0b01, 0b00, 0b00, 0b10, 0b10, 0b00, 0b10, 0b00,
            0b00, 0b11, 0b00, 0b00, 0b00, 0b10, 0b10, 0b11,
            0b00, 0b00, 0b00, 0b10, 0b11, 0b01, 0b00, 0b00,
            0b01, 0b11, 0b11, 0b10, 0b00, 0b10
        };
        */
        unsigned char sacch_raw[30];
        data->read((char*) sacch_raw, read_pos,  30);

        unsigned char sacch_descrambled[30];
        scrambler->scramble(sacch_raw, sacch_descrambled, 30);

        if (lich->getFunctionalType() == NXDN_USC_TYPE_SACCH_SF) {
            Sacch* sacch = Sacch::parse(sacch_descrambled);
            if (sacch != nullptr) {
                sacchCollector->push(sacch);
                if (sacchCollector->isComplete()) {
                    ((MetaWriter*) meta)->setSacch(sacchCollector->getSuperframe());
                    sacchCollector->reset();
                }
            } else {
                sacchCollector->reset();
            }
        }
        data->advance(read_pos, 30);

        unsigned char option = lich->getOption();

        // 2 * 2 voice frames, or maybe voice slots "stolen" by a FACCH1
        for (int i = 0; i < 2; i++) {
            unsigned char voice[72];
            data->read((char*) voice, read_pos, 72);

            unsigned char voice_descrambled[72];
            scrambler->scramble(voice, voice_descrambled, 72);

            // evaluate steal flag
            if ((option >> (1 - i)) & 1) {
                // only output actual voice frames if we are confident about the sync
                if (syncCount >= 1) {
                    // voice is only certain when we actually enter a voice frame
                    ((MetaWriter*) meta)->setSync("voice");

                    // take both voice frames in one go. why not?
                    unsigned char voice_frame[18] = { 0 };
                    for (int k = 0; k < 72; k++) {
                        voice_frame[k / 4] |= (voice_descrambled[k] & 3) << (6 - ((k % 4) * 2));
                    }

                    fwrite(voice_frame, sizeof(char), 18, stdout);
                    fflush(stdout);
                }
            } else {
                Facch1* facch1 = Facch1::parse(voice_descrambled);
                if (facch1 != nullptr) {
                    switch (facch1->getMessageType()) {
                        case NXDN_MESSAGE_TYPE_TX_RELEASE:
                            std::cerr << "FAACH1 signals TX release\n";
                            ((MetaWriter*) meta)->reset();
                            delete facch1;
                            return new SyncPhase();
                        case NXDN_MESSAGE_TYPE_IDLE:
                            break;
                        default:
                            std::cerr << "FACCH1 message type: " << +facch1->getMessageType() << "\n";
                    }
                    delete facch1;
                }
            }
            data->advance(read_pos, 72);
        }
    } else {
        // advance what's left
        data->advance(read_pos, 174);
    }

    return nullptr;
}