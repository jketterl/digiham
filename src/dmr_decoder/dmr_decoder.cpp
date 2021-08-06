#include "dmr_decoder.hpp"
#include "dmr_phase.hpp"
#include "dmr_meta.hpp"

using namespace Digiham::Dmr;

Decoder::Decoder(): Digiham::Decoder(new SyncPhase(), new MetaCollector()) {}

void Decoder::setSlotFilter(unsigned char filter) {
    slotFilter = filter;
    auto framePhase = dynamic_cast<FramePhase*>(currentPhase);
    if (framePhase != nullptr) {
        framePhase->setSlotFilter(slotFilter);
    }
}

void Decoder::setPhase(Digiham::Phase *phase) {
    Digiham::Decoder::setPhase(phase);
    auto framePhase = dynamic_cast<FramePhase*>(currentPhase);
    if (framePhase != nullptr) {
        framePhase->setSlotFilter(slotFilter);
    }
}