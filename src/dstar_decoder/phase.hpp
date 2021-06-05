#pragma once

#include "ringbuffer.hpp"
#include "header.hpp"
#include <cstddef>
#include <cstdint>

#define SYNC_SIZE 24
#define TERMINATOR_SIZE 48

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
                // the repeated 10s should always come ahead of the actual sync sequence, so we can use that to get
                // a more accurate sync and have the same length as the voice sync
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
            int getRequiredData() override { return 72 + 24 + 24; }
            Phase* process(Ringbuffer* data, size_t& read_pos) override;
        private:
            bool isSyncDue();
            int frameCount = 0;
            int syncMissing = 0;
            const uint8_t voice_sync[SYNC_SIZE] = {
                // 10 bits of 10
                1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
                // two times 1101000
                1, 1, 0, 1, 0, 0, 0,
                1, 1, 0, 1, 0, 0, 0,
            };
            const uint8_t terminator[TERMINATOR_SIZE] = {
                // 32 bits of 10
                1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
                // 000100110101111 + 0,
                0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0,
            };
    };

}