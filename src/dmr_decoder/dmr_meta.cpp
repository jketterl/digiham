#include <cassert>
#include "dmr_meta.hpp"
#include "dmr_phase.hpp"

using namespace Digiham::Dmr;

void Slot::setSync(int sync) {
    if (this->sync == sync) return;
    this->sync = sync;
    this->setDirty();
}

bool Slot::isDirty() {
    return dirty;
}

void Slot::setDirty() {
    dirty = true;
}

void Slot::setClean() {
    dirty = false;
}

void Slot::reset() {
    setSync(-1);
}

std::map<std::string, std::string> Slot::collect() {
    std::map<std::string, std::string> result;
    if (sync > 0) {
        result["sync"] = getSyncName();
    }
    return result;
}

std::string Slot::getSyncName() const {
    switch (sync) {
        case SYNCTYPE_DATA: return "data";
        case SYNCTYPE_VOICE: return "voice";
        default: return "unknown";
    }
}

MetaCollector::MetaCollector():
    Digiham::MetaCollector(),
    slots { new Slot(), new Slot() }
{}

std::string MetaCollector::getProtocol() {
    return "DMR";
}

MetaCollector::~MetaCollector() {
    for (int i = 0; i < 2; i++) {
        delete slots[i];
    }
}

void MetaCollector::withSlot(int slot, const std::function<void(Slot *)>& callback) {
    assert(slot < 2);
    callback(slots[slot]);
    sendMetaDataForSlot(slot);
}

void MetaCollector::sendMetaData() {
    for (int i = 0; i < 2; i++) {
        sendMetaDataForSlot(i);
    }
}

void MetaCollector::sendMetaDataForSlot(int slotIndex) {
    Slot* slot = slots[slotIndex];
    if (!slot->isDirty()) return;

    auto metadata = Digiham::MetaCollector::collect();
    metadata["slot"] = std::to_string(slotIndex);

    auto slotMetadata = slot->collect();
    metadata.insert(slotMetadata.begin(), slotMetadata.end());

    Digiham::MetaCollector::sendMetaData(metadata);
    slot->setClean();
}

void MetaCollector::reset() {
    hold();
    for (int i = 0; i < 2; i++) {
        slots[i]->reset();
    }
    release();
}