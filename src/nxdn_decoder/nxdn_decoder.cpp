#include "nxdn_decoder.hpp"
#include "nxdn_meta.hpp"
#include "nxdn_phase.hpp"

using namespace Digiham::Nxdn;

int main(int argc, char** argv) {
    Decoder decoder;
    return decoder.main(argc, argv);
}

Decoder::Decoder(): Digiham::Decoder(new MetaWriter()) {}

std::string Decoder::getName() {
    return "dstar_decoder";
}

Digiham::Phase* Decoder::getInitialPhase() {
    return new SyncPhase();
}
