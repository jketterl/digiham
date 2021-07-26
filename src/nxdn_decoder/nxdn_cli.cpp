#include "nxdn_cli.hpp"
#include "nxdn_decoder.hpp"

using namespace Digiham::Nxdn;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Cli::Cli(): Digiham::Cli(new Decoder()) {}

std::string Cli::getName() {
    return "nxdn_decoder";
}
