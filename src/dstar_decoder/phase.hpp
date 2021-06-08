#pragma once

#include "ringbuffer.hpp"
#include "header.hpp"
#include "scrambler.hpp"
#include "meta.hpp"
#include <cstddef>
#include <cstdint>
#include <string>

#define SYNC_SIZE 24
#define TERMINATOR_SIZE 48

namespace Digiham::DStar {

    class Phase {
        public:
            virtual int getRequiredData() = 0;
            virtual Phase* process(Ringbuffer* data, size_t& read_pos) = 0;
            void setMetaWriter(MetaWriter* meta);
        protected:
            MetaWriter* meta;
            const uint8_t header_sync[SYNC_SIZE] = {
                // part of the bitsync
                // the repeated 10s should always come ahead of the actual sync sequence, so we can use that to get
                // a more accurate sync and have the same length as the voice sync
                0, 1, 0, 1, 0, 1, 0, 1, 0,
                // actual sync
                1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0
            };
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

    class SyncPhase: public Phase {
        public:
            int getRequiredData() override { return SYNC_SIZE; }
            Phase* process(Ringbuffer* data, size_t& read_pos) override;
    };

    class HeaderPhase: public Phase {
        public:
            int getRequiredData() override { return Header::bits; }
            Phase* process(Ringbuffer* data, size_t& read_pos) override;
    };

    class VoicePhase: public Phase {
        public:
            VoicePhase();
            VoicePhase(int frameCount);
            ~VoicePhase();
            int getRequiredData() override { return 72 + 24 + 24; }
            Phase* process(Ringbuffer* data, size_t& read_pos) override;
        private:
            bool isSyncDue();
            void resetFrames();
            void collectDataFrame(unsigned char* data);
            void parseFrameData();
            int frameCount;
            int syncMissing = 0;
            Scrambler* scrambler;
            unsigned char collected_data[6] = { 0 };
            unsigned char message[20] = { 0 };
            unsigned char messageBlocks = 0;
            unsigned char header[41] = { 0 };
            unsigned char headerCount = 0;
            std::string simpleData = "";
    };

}