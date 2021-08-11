#include "nxdn_phase.hpp"
#include "facch1.hpp"
#include "types.hpp"

#include <cstring>

#include <iostream>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::Nxdn;

// -3. +1, -3, +3, -3, -3, +3, +3, -1, +3
const uint8_t Phase::frameSync[SYNC_SIZE] = { 3, 0, 3, 1, 3, 3, 1, 1, 2, 1 };

Digiham::Phase* SyncPhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    uint8_t* potential_sync = data->getReadPointer();

    if (hamming_distance(potential_sync, (uint8_t*) frameSync, SYNC_SIZE) <= 2) {
        std::cerr << "found a frame sync\n";
        //data->advance(SYNC_SIZE);
        return new FramedPhase();
    }

    // as long as we don't find any sync, move ahead, bit by bit
    data->advance(1);
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
    delete lich;
}

Digiham::Phase* FramedPhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    unsigned char* sync = data->getReadPointer();
    if (hamming_distance(sync, (uint8_t*) frameSync, SYNC_SIZE) <= 2) {
        // increase certainty, cap at 6
        if (++syncCount > 6) syncCount = 6;
    } else {
        if (--syncCount < 0) {
            std::cerr << "lost sync\n";
            ((MetaCollector*) meta)->reset();
            return new SyncPhase();
        }
    }
    data->advance(SYNC_SIZE);

    scrambler->reset();

    unsigned char* lich_raw = data->getReadPointer();
    data->advance(8);

    unsigned char lich_descrambled[8];
    scrambler->scramble(lich_raw, lich_descrambled, 8);

    Lich* new_lich = Lich::parse(lich_descrambled);
    if (new_lich != nullptr) {
        auto old = lich;
        lich = new_lich;
        delete old;
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
        unsigned char* sacch_raw = data->getReadPointer();

        unsigned char sacch_descrambled[30];
        scrambler->scramble(sacch_raw, sacch_descrambled, 30);

        if (lich->getFunctionalType() == NXDN_USC_TYPE_SACCH_SF) {
            Sacch* sacch = Sacch::parse(sacch_descrambled);
            if (sacch != nullptr) {
                sacchCollector->push(sacch);
                if (sacchCollector->isComplete()) {
                    SacchSuperframe* sacch = sacchCollector->getSuperframe();
                    if (sacch->getMessageType() == NXDN_MESSAGE_TYPE_VCALL) {
                        ((MetaCollector*) meta)->setSacch(sacch);
                    }
                    sacchCollector->reset();
                }
            }
        }
        data->advance(30);

        unsigned char option = lich->getOption();

        // 2 * 2 voice frames, or maybe voice slots "stolen" by a FACCH1
        for (int i = 0; i < 2; i++) {
            unsigned char* voice = data->getReadPointer();

            unsigned char voice_descrambled[72];
            scrambler->scramble(voice, voice_descrambled, 72);

            // evaluate steal flag
            if ((option >> (1 - i)) & 1) {
                // only output actual voice frames if we are confident about the sync
                if (syncCount >= 1) {
                    // voice is only certain when we actually enter a voice frame
                    ((MetaCollector*) meta)->setSync("voice");

                    // take both voice frames in one go. why not?
                    unsigned char* voice_frame = output->getWritePointer();
                    std::memset(voice_frame, 0, 18);
                    for (int k = 0; k < 72; k++) {
                        voice_frame[k / 4] |= (voice_descrambled[k] & 3) << (6 - ((k % 4) * 2));
                    }
                    output->advance(18);
                }
            } else {
                Facch1* facch1 = Facch1::parse(voice_descrambled);
                if (facch1 != nullptr) {
                    switch (facch1->getMessageType()) {
                        case NXDN_MESSAGE_TYPE_TX_RELEASE:
                            std::cerr << "FAACH1 signals TX release\n";
                            ((MetaCollector*) meta)->reset();
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
            data->advance(72);
        }
    } else {
        // advance what's left
        data->advance(174);
    }

    return nullptr;
}