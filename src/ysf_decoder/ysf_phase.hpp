#pragma once

#include "phase.hpp"
#include "fich.hpp"
#include "data.hpp"

#define SYNC_SIZE 20
#define FICH_SIZE 100
#define PAYLOAD_SIZE 360

// SYNC_SIZE + FICH_SIZE + PAYLOAD_SIZE
#define FRAME_SIZE 480

namespace Digiham::Ysf {

    class Phase: public Digiham::Phase {
        protected:
            bool isSync(unsigned char* input);
        private:
            //D471C9634D
            const unsigned char ysf_sync[SYNC_SIZE] =  { 3,1,1,0,1,3,0,1,3,0,2,1,1,2,0,3,1,0,3,1 };
    };

    class SyncPhase: public Phase {
        public:
            int getRequiredData() override;
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) override;
    };

    class FramePhase: public Phase {
        public:
            ~FramePhase() override;
            int getRequiredData() override;
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) override;
        private:
            void decodeV1VoicePayload(unsigned char* in, unsigned char* out);
            void decodeV2VoicePayload(unsigned char* in, unsigned char* out);
            void decodeTribits(uint8_t* in, uint8_t* out, uint8_t length);
            void interleaveV2VoicePayload(uint8_t payload[7], uint8_t result[7]);
            void decodeV2DataChannel(unsigned char* in, unsigned char frameNumer);
            void decodeFRVoicePayload(unsigned char* in, unsigned char* out);
            unsigned char* decodeHeaderDataChannel(unsigned char* in);
            std::string treatYsfString(const char* input, size_t length);

            const uint8_t tribit_majority_table[8] = { 0, 0, 0, 1, 0, 1, 1, 1 };
            const uint8_t v2_voice_mapping[49] = {
                // found in https://github.com/HB9UF/gr-ysf/issues/12
                0, 3, 6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 41, 43, 45, 47,
                1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 42, 44, 46, 48,
                2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38,
            };

            int syncCount = 0;
            Fich* runningFich = nullptr;
            DataCollector* dataCollector = new DataCollector();
            bool expectSubFrame = false;
    };

}