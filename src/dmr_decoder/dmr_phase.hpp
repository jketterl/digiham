#pragma once

#include "phase.hpp"
#include "embedded.hpp"
#include "talkeralias.hpp"

#define SYNC_SIZE 24
#define CACH_SIZE 12

// full frame including CACH, SYNC and the two payload blocks
#define FRAME_SIZE 144

#define SYNCTYPE_DATA 1
#define SYNCTYPE_VOICE 2

#define META_TYPE_DIRECT 1
#define META_TYPE_GROUP 2

namespace Digiham::Dmr {

    class Phase: public Digiham::Phase {
        protected:
            int getSyncType(unsigned char* potentialSync);

            const unsigned char dmr_bs_data_sync[SYNC_SIZE] =  { 3,1,3,3,3,3,1,1,1,3,3,1,1,3,1,1,3,1,3,3,1,1,3,1 };
            const unsigned char dmr_bs_voice_sync[SYNC_SIZE] = { 1,3,1,1,1,1,3,3,3,1,1,3,3,1,3,3,1,3,1,1,3,3,1,3 };
            const unsigned char dmr_ms_data_sync[SYNC_SIZE] =  { 3,1,1,1,3,1,1,3,3,3,1,3,1,3,3,3,3,1,1,3,1,1,1,3 };
            const unsigned char dmr_ms_voice_sync[SYNC_SIZE] = { 1,3,3,3,1,3,3,1,1,1,3,1,3,1,1,1,1,3,3,1,3,3,3,1 };

            // in DMR frames, the sync is in the middle. therefor, we need to be able to look at previous data once we
            // find a sync.
            // The data of one channel is 54 symbols, and the length of the CACH in basestation transmissions is 12.
            unsigned int syncOffset = 54 + CACH_SIZE;
    };

    class SyncPhase: public Phase {
        public:
            int getRequiredData() override;
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) override;
    };

    class FramePhase: public Phase {
        public:
            FramePhase();
            ~FramePhase() override;
            int getRequiredData() override;
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) override;
            void setSlotFilter(unsigned char filter);
        private:
            void handleLc(Lc* lc);
            int syncCount = 0;
            int slot = -1;
            int slotStability = 0;
            int syncTypes[2] = {-1, -1};
            int slotSyncCount[2] = {0, 0};
            EmbeddedCollector* embCollectors[2];
            TalkerAliasCollector* talkerAliasCollector[2];
            int activeSlot = -1;
            unsigned char slotFilter = 3;
            unsigned char superframeCounter[2] = {0, 0};
    };

}