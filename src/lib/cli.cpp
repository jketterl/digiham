#include "cli.hpp"
#include <iostream>
#include <getopt.h>

using namespace Digiham;

template <typename T>
Cli<T>::Cli(Csdr::Module<T, T>* decoder):
    decoder(decoder),
    ringbuffer(new Csdr::Ringbuffer<T>(RINGBUFFER_SIZE))
{}

template <typename T>
Cli<T>::~Cli() {
    delete decoder;
    delete ringbuffer;
}

template <typename T>
int Cli<T>::main(int argc, char** argv) {
    if (!parseOptions(argc, argv)) {
        return 0;
    }

    decoder->setReader(new Csdr::RingbufferReader<T>(ringbuffer));
    decoder->setWriter(new Csdr::StdoutWriter<T>());

    while (read()) {
        while (decoder->canProcess()) {
            decoder->process();
        }
    }
    return 0;
}

template <typename T>
void Cli<T>::printUsage() {
    // don't show fifo in the usage if it doesn't work
    std::string fifoLine = "";
    if (dynamic_cast<Decoder*>(decoder)) {
        fifoLine = " -f, --fifo          send metadata to this file\n";
    }
    std::cerr <<
              getName() << " version " << VERSION << "\n\n" <<
              "Usage: " << getName() << " [options]\n\n" <<
              "Available options:\n" <<
              " -h, --help          show this message\n" <<
              " -v, --version       print version and exit\n" <<
              fifoLine;
}

template <typename T>
void Cli<T>::printVersion() {
    std::cout << getName() << " version " << VERSION << "\n";
}

template <typename T>
bool Cli<T>::parseOptions(int argc, char** argv) {
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
                auto metaDecoder = dynamic_cast<Decoder*>(decoder);
                if (metaDecoder == NULL) {
                    std::cerr << "meta fifo is not supported\n";
                    break;
                }
                std::cerr << "meta fifo: " << optarg << "\n";
                metaDecoder->setMetaFile(fopen(optarg, "w"));
                break;
        }
    }

    return true;
}

template <typename T>
bool Cli<T>::read() {
    int r = fread(ringbuffer->getWritePointer(), sizeof(T), std::min(ringbuffer->writeable(), (size_t) BUF_SIZE), stdin);
    ringbuffer->advance(r);
    return r > 0;
}

namespace Digiham {
    template class Cli<unsigned char>;
    template class Cli<float>;
}