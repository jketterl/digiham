#include "rrc_filter_cli.hpp"
#include "rrc_filter.hpp"

using namespace Digiham::RrcFilter;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

std::string Cli::getName() {
    return "rrc_filter";
}

std::stringstream Cli::getUsageString() {
    std::stringstream result = Digiham::Cli<float, float>::getUsageString();
    result <<
           " -n, --narrow        use narrow (6.25kHz) filter version (default: wide / 12.5kHz)\n";
    return result;
}

std::vector<struct option> Cli::getOptions() {
    std::vector<struct option> options = Digiham::Cli<float, float>::getOptions();
    options.push_back({"narrow", no_argument, NULL, 'n'});
    return options;
}

bool Cli::receiveOption(int c, char *optarg) {
    switch (c) {
        case 'n':
            narrow = true;
            break;
        default:
            return Digiham::Cli<float, float>::receiveOption(c, optarg);
    }
    return true;
}

Csdr::Module<float, float>* Cli::buildModule() {
    if (narrow) {
        return new NarrowRrcFilter();
    } else {
        return new WideRrcFilter();
    }
}