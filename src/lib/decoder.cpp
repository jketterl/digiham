#include "decoder.hpp"
#include "phase.hpp"
#include "meta.hpp"

using namespace Digiham;

Decoder::Decoder(Phase* initialPhase, MetaCollector* collector):
    currentPhase(initialPhase),
    metaCollector(collector)
{}

Decoder::Decoder(Phase* initialPhase):
    Decoder(initialPhase, nullptr)
{}

Decoder::~Decoder() {
    delete currentPhase;
    delete metaCollector;
}

bool Decoder::canProcess() {
    std::lock_guard<std::mutex> lock(processMutex);
    return reader->available() > currentPhase->getRequiredData();
}

void Decoder::process() {
    std::lock_guard<std::mutex> lock(processMutex);
    Phase* next = currentPhase->process(reader, writer);
    if (next != nullptr) {
        setPhase(next);
    }
}

void Decoder::setMetaWriter(MetaWriter *writer) {
    if (metaCollector == nullptr) {
        delete writer;
        return;
    }
    metaCollector->setWriter(writer);
}

void Decoder::setPhase(Phase* phase) {
    if (phase == currentPhase) return;
    delete currentPhase;
    currentPhase = phase;
    currentPhase->setMetaCollector(metaCollector);
}