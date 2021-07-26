#include "dstar_decoder.hpp"
#include "dstar_phase.hpp"
#include "dstar_meta.hpp"

using namespace Digiham::DStar;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Decoder::Decoder(): Digiham::Decoder(new MetaWriter(), new SyncPhase()) {}

Cli::Cli(): Digiham::Cli(new Decoder()) {}

std::string Cli::getName() {
    return "dstar_decoder";
}
