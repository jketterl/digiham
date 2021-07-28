#include "cli.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>

using namespace Digiham;

template <typename T, typename U>
Cli<T, U>::Cli():
    ringbuffer(new Csdr::Ringbuffer<T>(RINGBUFFER_SIZE))
{}

template <typename T, typename U>
Cli<T, U>::~Cli() {
    delete ringbuffer;
}

template <typename T, typename U>
int Cli<T, U>::main(int argc, char** argv) {
    if (!parseOptions(argc, argv)) {
        return 0;
    }

    auto module = buildModule();

    module->setReader(new Csdr::RingbufferReader<T>(ringbuffer));
    module->setWriter(new Csdr::StdoutWriter<U>());

    while (read()) {
        while (module->canProcess()) {
            module->process();
        }
    }

    delete module;
    return 0;
}

template <typename T, typename U>
std::vector<struct option> Cli<T, U>::getOptions() {
    return {
        {"version", no_argument,       NULL, 'v'},
        {"help",    no_argument,       NULL, 'h'},
    };
}

template <typename T, typename U>
std::stringstream Cli<T, U>::getUsageString() {
    std::stringstream result;
    result <<
           getName() << " version " << VERSION << "\n\n" <<
           "Usage: " << getName() << " [options]\n\n" <<
           "Available options:\n" <<
           " -h, --help          show this message\n" <<
           " -v, --version       print version and exit\n";
    return result;
}

template <typename T, typename U>
void Cli<T, U>::printVersion() {
    std::cout << getName() << " version " << VERSION << "\n";
}

template <typename T, typename U>
bool Cli<T, U>::parseOptions(int argc, char** argv) {
    std::vector<struct option> long_options = getOptions();
    long_options.push_back({ NULL, 0, NULL, 0 });

    std::vector<std::string> short_options;
    std::transform(
            long_options.begin(),
            long_options.end(),
            std::inserter(short_options, short_options.end()),
            [](struct option opt){
                return std::string(1, opt.val) + (opt.has_arg == required_argument ? ":" : "");
            }
    );
    std::string short_options_string = std::accumulate(short_options.begin(), short_options.end(), std::string(""));

    int c;
    while ((c = getopt_long(argc, argv, short_options_string.c_str(), long_options.data(), NULL)) != -1) {
        bool r = receiveOption(c, optarg);
        if (!r) return r;
    }
    return true;
}

template <typename T, typename U>
bool Cli<T, U>::receiveOption(int c, char *optarg) {
    switch (c) {
        case 'v':
            printVersion();
            return false;
        case 'h':
        default:
            std::cerr << getUsageString().str();
            return false;
    }
}

template <typename T, typename U>
bool Cli<T, U>::read() {
    int r = fread(ringbuffer->getWritePointer(), sizeof(T), std::min(ringbuffer->writeable(), (size_t) BUF_SIZE), stdin);
    ringbuffer->advance(r);
    return r > 0;
}

namespace Digiham {
    template class Cli<unsigned char, unsigned char>;
    template class Cli<float, float>;
    template class Cli<float, unsigned char>;
}

std::stringstream DecoderCli::getUsageString() {
    std::stringstream result = Cli<unsigned char, unsigned char>::getUsageString();
    result << " -f, --fifo          send metadata to this file\n";
    return result;
}

std::vector<struct option> DecoderCli::getOptions() {
    std::vector<struct option> options = Cli<unsigned char, unsigned char>::getOptions();
    options.push_back({"fifo", required_argument, NULL, 'f'});
    return options;
}

bool DecoderCli::receiveOption(int c, char *optarg) {
    switch (c) {
        case 'f': {
            std::cerr << "meta fifo: " << optarg << "\n";
            metaFile = fopen(optarg, "w");
            break;
        }
        default:
            return Cli<unsigned char, unsigned char>::receiveOption(c, optarg);
    }
    return true;
}