#include "nxdn_decoder.hpp"
#include "nxdn_meta.hpp"
#include "nxdn_phase.hpp"

using namespace Digiham::Nxdn;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Decoder::Decoder(): Digiham::Decoder(new MetaWriter(), new SyncPhase) {}

Cli::Cli(): Digiham::Cli(new Decoder()) {}

std::string Cli::getName() {
    return "nxdn_decoder";
}
