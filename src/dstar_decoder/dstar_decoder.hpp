#pragma once

#include "header.hpp"
#include "phase.hpp"
#include "ringbuffer.hpp"
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
            bool read();
            void searchSync();
            Ringbuffer* ringbuffer;
            size_t read_pos = 0;
            Phase* currentPhase;
    };

}