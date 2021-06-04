#pragma once

#include "ringbuffer.hpp"
#include "header.hpp"
#include <cstddef>
#include <cstdint>

#define SYNC_SIZE 24

namespace Digiham::DStar {

    class Phase {
        public:
            virtual int getRequiredData() = 0;
            virtual Phase* process(Ringbuffer* data, size_t& read_pos) = 0;
    };

    class SyncPhase: public Phase {
        public:
            int getRequiredData() override { return SYNC_SIZE; }
            Phase* process(Ringbuffer* data, size_t& read_pos) override;
        private:
            const uint8_t header_sync[SYNC_SIZE] = {
                // part of the bitsync
                0, 1, 0, 1, 0, 1, 0, 1, 0,
                // actual sync
                1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0
            };
    };

    class HeaderPhase: public Phase {
        public:
            int getRequiredData() override { return Header::bits; }
            Phase* process(Ringbuffer* data, size_t& read_pos) override;
    };

    class VoicePhase: public Phase {
        public:
            int getRequiredData() override { return 72; }
            Phase* process(Ringbuffer* data, size_t& read_pos) override;
    };

}