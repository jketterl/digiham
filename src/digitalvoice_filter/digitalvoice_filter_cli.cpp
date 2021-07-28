#include "digitalvoice_filter.hpp"
#include "digitalvoice_filter_cli.hpp"

using namespace Digiham::DigitalVoice;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Csdr::Module<short, short>* Cli::buildModule() {
    return new DigitalVoiceFilter();
}

std::string Cli::getName() {
    return "digitalvoice_filter";
}