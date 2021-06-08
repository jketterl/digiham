#include "dstar_decoder.hpp"
#include <iostream>
#include <cstring>
#include <getopt.h>

using namespace Digiham::DStar;

int main(int argc, char** argv) {
    Decoder decoder;
    return decoder.main(argc, argv);
}

int Decoder::main(int argc, char** argv) {
    if (!parseOptions(argc, argv)) {
        return 0;
    }

    setPhase(new SyncPhase());
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
        "dstar_decoder version " << VERSION << "\n\n" <<
        "Usage: dstar_decoder [options]\n\n" <<
        "Available options:\n" <<
        " -h, --help              show this message\n" <<
        " -v, --version           print version and exit\n" <<
        " -f, --fifo          send metadata to this file\n";
}

void Decoder::printVersion() {
    std::cout << "dstar_decoder version " << VERSION << "\n";
}

bool Decoder::parseOptions(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {"fifo", required_argument, NULL, 'f'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "yvhs:d", long_options, NULL)) != -1 ) {
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

Decoder::Decoder():
    ringbuffer(new Ringbuffer(RINGBUFFER_SIZE)),
    meta(new MetaWriter())
{}

Decoder::~Decoder() {
    delete ringbuffer;
    delete meta;
}