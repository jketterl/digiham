#include "dstar_decoder.hpp"
#include <iostream>
#include <cstring>

using namespace Digiham::DStar;

int main(int argc, char** argv) {
    Decoder decoder;
    return decoder.main(argc, argv);
}

int Decoder::main(int argc, char** argv) {
    int r;
    currentPhase = new SyncPhase();
    while (read()) {
        while (ringbuffer->available(read_pos) > currentPhase->getRequiredData()) {
            Phase* next = currentPhase->process(ringbuffer, read_pos);
            if (next != nullptr && next != currentPhase) {
                delete currentPhase;
                currentPhase = next;
            }
        }
    }
    return 0;
}

bool Decoder::read() {
    int r = fread(ringbuffer->getWritePointer(), 1, std::min(ringbuffer->writeable(), (size_t) BUF_SIZE), stdin);
    ringbuffer->advance(r);
    return r > 0;
}

Decoder::Decoder() {
    ringbuffer = new Ringbuffer(RINGBUFFER_SIZE);
}

Decoder::~Decoder() {
    delete ringbuffer;
}