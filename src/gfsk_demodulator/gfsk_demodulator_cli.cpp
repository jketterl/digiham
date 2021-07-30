#include "gfsk_demodulator_cli.hpp"

using namespace Digiham::Fsk;

int main(int argc, char** argv) {
    GfskCli runner;
    return runner.main(argc, argv);
}

std::string GfskCli::getName() {
    return "gfsk_demodulator";
}

Csdr::Module<float, unsigned char>* GfskCli::buildModule() {
    return new GfskDemodulator(samplesPerSymbol);
};

std::stringstream GfskCli::getUsageString() {
    std::stringstream result = Digiham::Cli<float, unsigned char>::getUsageString();
    result <<
           " -s, --samples       samples per symbol ( = audio sample rate / symbol rate; default: 10)\n";
    return result;
}

std::vector<struct option> GfskCli::getOptions() {
    std::vector<struct option> options = Digiham::Cli<float, unsigned char>::getOptions();
    options.push_back({"samples", required_argument, NULL, 's'});
    return options;
}

bool GfskCli::receiveOption(int c, char *optarg) {
    switch (c) {
        case 's':
            samplesPerSymbol = std::strtoul(optarg, NULL, 10);
            break;
        default:
            return Digiham::Cli<float, unsigned char>::receiveOption(c, optarg);
    }
    return true;
}