#include "cli.hpp"
#include <iostream>
#include <getopt.h>

using namespace Digiham;

Cli::Cli(Decoder* decoder):
        decoder(decoder),
        ringbuffer(new Csdr::Ringbuffer<unsigned char>(RINGBUFFER_SIZE))
{}

Cli::~Cli() {
    delete decoder;
    delete ringbuffer;
}

int Cli::main(int argc, char** argv) {
    if (!parseOptions(argc, argv)) {
        return 0;
    }

    decoder->setReader(new Csdr::RingbufferReader<unsigned char>(ringbuffer));
    decoder->setWriter(new Csdr::StdoutWriter<unsigned char>());

    while (read()) {
        while (decoder->canProcess()) {
            decoder->process();
        }
    }
    return 0;
}

void Cli::printUsage() {
    std::cerr <<
              getName() << " version " << VERSION << "\n\n" <<
              "Usage: " << getName() << " [options]\n\n" <<
              "Available options:\n" <<
              " -h, --help              show this message\n" <<
              " -v, --version           print version and exit\n" <<
              " -f, --fifo          send metadata to this file\n";
}

void Cli::printVersion() {
    std::cout << getName() << " version " << VERSION << "\n";
}

bool Cli::parseOptions(int argc, char** argv) {
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
                decoder->setMetaFile(fopen(optarg, "w"));
                break;
        }
    }

    return true;
}

bool Cli::read() {
    int r = fread(ringbuffer->getWritePointer(), 1, std::min(ringbuffer->writeable(), (size_t) BUF_SIZE), stdin);
    ringbuffer->advance(r);
    return r > 0;
}