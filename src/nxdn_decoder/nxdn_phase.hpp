#pragma once

#include "phase.hpp"
#include "ringbuffer.hpp"

#define SYNC_SIZE 10

namespace Digiham::Nxdn {

    class SyncPhase: public Digiham::Phase {
        public:
            int getRequiredData() override { return 10; }
            Digiham::Phase* process(Ringbuffer* data, size_t& read_pos) override;
        private:
            static const uint8_t frameSync[SYNC_SIZE];
    };

}