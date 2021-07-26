#include "dstar_cli.hpp"

#include "dstar_decoder.hpp"

using namespace Digiham::DStar;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Cli::Cli(): Digiham::Cli(new Decoder()) {}

std::string Cli::getName() {
    return "dstar_decoder";
}
