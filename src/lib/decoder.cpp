#include "decoder.hpp"
#include <iostream>
#include <getopt.h>

using namespace Digiham;

int Decoder::main(int argc, char** argv) {
    if (!parseOptions(argc, argv)) {
        return 0;
    }

    setPhase(getInitialPhase());
    while (read()) {
        while (ringbuffer->available(read_pos) > currentPhase->getRequiredData()) {
            Phase* next = currentPhase->process(ringbuffer, read_pos);
            if (next != nullptr) {
                setPhase(next);
            }
        }
    }
    return 0;
}

void Decoder::setPhase(Phase* phase) {
    if (phase == currentPhase) return;
    if (currentPhase != nullptr) {
        delete currentPhase;
    }
    currentPhase = phase;
    currentPhase->setMetaWriter(meta);
}

void Decoder::printUsage() {
    std::cerr <<
        getName() << " version " << VERSION << "\n\n" <<
        "Usage: " << getName() << " [options]\n\n" <<
        "Available options:\n" <<
        " -h, --help              show this message\n" <<
        " -v, --version           print version and exit\n" <<
        " -f, --fifo          send metadata to this file\n";
}

void Decoder::printVersion() {
    std::cout << getName() << " version " << VERSION << "\n";
}

bool Decoder::parseOptions(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {"fifo", required_argument, NULL, 'f'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "vhf:", long_options, NULL)) != -1 ) {
        switch (c) {
            case 'v':
                printVersion();
                return false;
            case 'h':
                printUsage();
                return false;
            case 'f':
                std::cerr << "meta fifo: " << optarg << "\n";
                meta->setFile(fopen(optarg, "w"));
                break;
        }
    }

    return true;
}

bool Decoder::read() {
    int r = fread(ringbuffer->getWritePointer(), 1, std::min(ringbuffer->writeable(), (size_t) BUF_SIZE), stdin);
    ringbuffer->advance(r);
    return r > 0;
}

Decoder::Decoder(MetaWriter* meta):
    ringbuffer(new Ringbuffer(RINGBUFFER_SIZE)),
    meta(meta)
{}

Decoder::~Decoder() {
    delete ringbuffer;
    delete meta;
}