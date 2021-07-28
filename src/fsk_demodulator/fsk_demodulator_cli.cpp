#include "fsk_demodulator.hpp"
#include "fsk_demodulator_cli.hpp"

using namespace Digiham::FskDemodulator;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Csdr::Module<float, unsigned char>* Cli::buildModule() {
    // TODO: parse constructor arguments from CLI
    return new FskDemodulator(samplesPerSymbol, invert);
}

std::string Cli::getName() {
    return "fsk_demodulator";
}

std::stringstream Cli::getUsageString() {
    std::stringstream result = Digiham::Cli<float, unsigned char>::getUsageString();
    result <<
           " -s, --samples       samples per symbol ( = audio sample rate / symbol rate; default: 40)\n" <<
           " -i, --invert        invert bits (used e.g in pocsag)\n";
    return result;
}

std::vector<struct option> Cli::getOptions() {
    std::vector<struct option> options = Digiham::Cli<float, unsigned char>::getOptions();
    options.push_back({"samples", required_argument, NULL, 's'});
    options.push_back({"invert", no_argument, NULL, 'i'});
    return options;
}

bool Cli::receiveOption(int c, char *optarg) {
    switch (c) {
        case 's':
            samplesPerSymbol = std::strtoul(optarg, NULL, 10);
            break;
        case 'i':
            invert = true;
            break;
        default:
            return Digiham::Cli<float, unsigned char>::receiveOption(c, optarg);
    }
    return true;
}