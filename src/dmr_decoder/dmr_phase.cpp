#include <cstring>
#include "dmr_phase.hpp"
#include "emb.hpp"
#include "cach.hpp"

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::Dmr;

int Phase::getSyncType(unsigned char *potentialSync) {
    if (hamming_distance((uint8_t*) potentialSync, (uint8_t*) dmr_bs_data_sync, SYNC_SIZE) <= 3) {
        return SYNCTYPE_DATA;
    }
    if (hamming_distance((uint8_t*) potentialSync, (uint8_t*) dmr_bs_voice_sync, SYNC_SIZE) <= 3) {
        return SYNCTYPE_VOICE;
    }
    if (hamming_distance((uint8_t*) potentialSync, (uint8_t*) dmr_ms_data_sync, SYNC_SIZE) <= 3) {
        return SYNCTYPE_DATA;
    }
    if (hamming_distance((uint8_t*) potentialSync, (uint8_t*) dmr_ms_voice_sync, SYNC_SIZE) <= 3) {
        return SYNCTYPE_VOICE;
    }

    return -1;
}

int SyncPhase::getRequiredData() {
    return SYNC_SIZE + syncOffset;
};

Digiham::Phase* SyncPhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    int syncType = getSyncType(data->getReadPointer() + syncOffset);
    if (syncType > 0) return new FramePhase();

    // as long as we don't find any sync, move ahead, symbol by symbol
    data->advance(1);
    // tell decoder that we'll continue
    return this;
}

FramePhase::FramePhase(): syncCount(0) {}

int FramePhase::getRequiredData() {
    return FRAME_SIZE;
}

Digiham::Phase *FramePhase::process(Csdr::Reader<unsigned char> *data, Csdr::Writer<unsigned char> *output) {
    int syncType = getSyncType(data->getReadPointer() + syncOffset);
    if (syncType > 0) {
        // increase sync count, cap at 5
        if (++syncCount > 5) syncCount = 5;
    } else {
        uint16_t emb_data = 0;

        // try to decode as embedded signalling
        for (int i = 0; i < 2; i++) {
            unsigned int offset = syncOffset + i * 20;
            unsigned char* raw = data->getReadPointer() + offset;
            for (int k = 0; k < 4; k++) {
                emb_data = (emb_data << 2) | raw[k];
            }
        }
        Emb* emb = Emb::parse(emb_data);
        if (emb != nullptr) {
            // if the EMB decoded correctly, that counts towards the sync :)
            if (++syncCount > 5) syncCount = 5;
        } else {
            // no sync and no EMB, decrease sync counter
            if (--syncCount < 0) {
                //((MetaCollector*) meta)->reset();
                return new SyncPhase();
            }
        }
    }

    Cach* cach = Cach::parse(data->getReadPointer());
    // slots should always be alternating, but may be overridden by 100% correct tact
    unsigned char next = slot ^ 1;
    if (cach->hasTact()) {
        if (cach->getTact()->getSlot() != next) {
            if (slotStability < 5) {
                slotStability = 0;
                slot = cach->getTact()->getSlot();
            } else {
                if (slotStability-- < -100) slotStability = -100;
                if (slot != -1) {
                    slot = next;
                }
            }
        } else {
            if (++slotStability > 100) slotStability = 100;
            slot = next;
        }
    } else if (slot != -1) {
        if (slotStability-- < -100) slotStability = -100;
        slot = next;
    }

    if (slot != -1) {
        if (syncType > 0) syncTypes[slot] = syncType;

        if (syncTypes[slot] == SYNCTYPE_VOICE) {
            // don't output anything if the slot is muted
            // don't output anything if the other slot is active
            if (((slot + 1) & slotFilter) && (activeSlot == -1 || activeSlot == slot)) {
                activeSlot = slot;
                // extract payload data
                unsigned char* payload = output->getWritePointer();
                std::memset(payload, 0, 27);
                // first half
                unsigned char* payloadRaw = data->getReadPointer() + 12;
                for (int i = 0; i < 54; i++) {
                    payload[i / 4] |= (payloadRaw[i] & 3) << (6 - 2 * (i % 4));
                }

                // second half
                payloadRaw += 54 + SYNC_SIZE;
                for (int i = 0; i < 54; i++) {
                    payload[(i + 54) / 4] |= (payloadRaw[i] & 3) << (6 - 2 * ((i + 54) % 4));
                }
                output->advance(27);
            }
        } else {
            if (activeSlot == slot) {
                activeSlot = -1;
            }
        }
    }


    data->advance(FRAME_SIZE);
    return this;
}
