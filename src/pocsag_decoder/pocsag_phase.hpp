#pragma once

#include "phase.hpp"
#include "message.hpp"
#include "meta.hpp"

#define SYNC_SIZE 32

namespace Digiham::Pocsag {

    class Phase: public Digiham::Phase {
        protected:
            bool hasSync(unsigned char* data);
        private:
            const uint8_t pocsag_sync[SYNC_SIZE] = { 0,1,1,1,1,1,0,0,1,1,0,1,0,0,1,0,0,0,0,1,0,1,0,1,1,1,0,1,1,0,0,0 };
    };

    class SyncPhase: public Phase {
        public:
            int getRequiredData() override;
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) override;
    };

    class CodewordPhase: public Phase {
        public:
            ~CodewordPhase() override;
            int getRequiredData() override;
            Digiham::Phase* process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) override;
            void setSerializer(Digiham::Serializer* serializer);
        private:
            void writeMessage(Message* message);
            // we start after one sync has already been found
            int syncCount = 1;
            int codewordCounter = 0;
            Message* currentMessage = nullptr;
            Serializer* serializer = nullptr;
    };

}