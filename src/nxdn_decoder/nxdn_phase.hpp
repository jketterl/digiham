#pragma once

#include "phase.hpp"
#include "scrambler.hpp"
#include "lich.hpp"
#include "sacch.hpp"
#include "nxdn_meta.hpp"

#include <csdr/reader.hpp>

#define SYNC_SIZE 10
// 384 bits or 192 symbols
#define FRAME_SIZE 192

namespace Digiham::Nxdn {

    class Phase: public Digiham::Phase {
        protected:
            static const uint8_t frameSync[SYNC_SIZE];
    };

    class SyncPhase: public Phase {
        public:
            int getRequiredData() override { return 10; }
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data) override;
    };

    class FramedPhase: public Phase {
        public:
            FramedPhase();
            ~FramedPhase();
            int getRequiredData() override { return FRAME_SIZE; }
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data) override;
        private:
            int syncCount = 0;
            Scrambler* scrambler;
            Lich* lich = nullptr;
            SacchSuperframeCollector* sacchCollector;
    };

}