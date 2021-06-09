#pragma once

#include "phase.hpp"
#include "ringbuffer.hpp"

namespace Digiham::Nxdn {

    class SyncPhase: public Digiham::Phase {
        public:
            int getRequiredData() override { return 10; }
            Digiham::Phase* process(Ringbuffer* data, size_t& read_pos) override;
    };

}