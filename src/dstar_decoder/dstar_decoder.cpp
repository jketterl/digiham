#include "dstar_decoder.hpp"
#include "dstar_phase.hpp"
#include "dstar_meta.hpp"

using namespace Digiham::DStar;

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
