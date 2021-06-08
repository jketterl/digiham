#pragma once

#include "header.hpp"
#include "phase.hpp"
#include "ringbuffer.hpp"
#include "meta.hpp"
#include <cstddef>

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

namespace Digiham::DStar {

    class Decoder {
        public:
            Decoder();
            ~Decoder();
            int main (int argc, char** argv);
        private:
            void printUsage();
            void printVersion();
            bool parseOptions(int argc, char** argv);
            bool read();
            void searchSync();
            void setPhase(Phase* phase);
            MetaWriter* meta;
            Ringbuffer* ringbuffer;
            size_t read_pos = 0;
            Phase* currentPhase = nullptr;
    };

}