#pragma once

#include "meta.hpp"
#include "phase.hpp"
#include <string>

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

namespace Digiham {

    class Decoder {
        public:
            int main (int argc, char** argv);
            Decoder(MetaWriter* meta);
            ~Decoder();
        protected:
            virtual std::string getName() = 0;
            virtual void printUsage();
            virtual void printVersion();
            virtual bool parseOptions(int argc, char** argv);
            virtual Phase* getInitialPhase() = 0;
        private:
            bool read();
            void setPhase(Phase* phase);
            MetaWriter* meta;
            Ringbuffer* ringbuffer;
            size_t read_pos = 0;
            Phase* currentPhase = nullptr;
    };

}