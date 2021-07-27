#include "decoder.hpp"
#include "phase.hpp"
#include "meta.hpp"

using namespace Digiham;

Decoder::Decoder(MetaWriter* meta, Phase* initialPhase):
    meta(meta),
    currentPhase(initialPhase)
{}

Decoder::~Decoder() {
    delete meta;
}

bool Decoder::canProcess() {
    return reader->available() > currentPhase->getRequiredData();
}

void Decoder::process() {
    Phase* next = currentPhase->process(reader, writer);
    if (next != nullptr) {
        setPhase(next);
    }
}

void Decoder::setPhase(Phase* phase) {
    if (phase == currentPhase) return;
    delete currentPhase;
    currentPhase = phase;
    currentPhase->setMetaWriter(meta);
}

void Decoder::setMetaFile(FILE *file) {
    meta->setFile(file);
}
