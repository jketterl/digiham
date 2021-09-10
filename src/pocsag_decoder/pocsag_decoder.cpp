#include "pocsag_decoder.hpp"
#include "pocsag_phase.hpp"

using namespace Digiham::Pocsag;

Decoder::Decoder(Serializer *serializer): Digiham::Decoder(new SyncPhase()), serializer(serializer) {}

Decoder::Decoder(): Decoder(new StringSerializer()) {}

void Decoder::setPhase(Digiham::Phase *phase) {
    Digiham::Decoder::setPhase(phase);
    auto cwPhase = dynamic_cast<CodewordPhase*>(phase);
    if (cwPhase != nullptr) {
        cwPhase->setSerializer(serializer);
    }
}