#include "nxdn_meta.hpp"
#include "types.hpp"

using namespace Digiham::Nxdn;

std::map<std::string, std::string> MetaCollector::collect() {
    auto metadata = Digiham::MetaCollector::collect();

    if (!sync.empty()) {
        metadata["sync"] = sync;
    }

    if (!type.empty()) {
        metadata["type"] = type;
    }

    if (source != 0) {
        metadata["source"] = std::to_string(source);
    }

    if (destination != 0) {
        metadata["destination"] = std::to_string(destination);
    }

    return metadata;
}

std::string MetaCollector::getProtocol() {
    return "NXDN";
}

void MetaCollector::setSync(std::string sync) {
    if (this->sync == sync) return;
    this->sync = sync;
    sendMetaData();
}

void MetaCollector::setType(std::string type) {
    if (this->type == type) return;
    this->type = type;
    sendMetaData();
}

void MetaCollector::setSource(uint16_t source) {
    if (this->source == source) return;
    this->source = source;
    sendMetaData();
}

void MetaCollector::setDestination(uint16_t destination) {
    if (this->destination == destination) return;
    this->destination = destination;
    sendMetaData();
}

void MetaCollector::setFromSacch(SacchSuperframe* sacch) {
    if (sacch->getMessageType() == NXDN_MESSAGE_TYPE_VCALL) {
        if (sacch->getCallType() == NXDN_CALL_TYPE_CONFERENCE) {
            setType("conference");
        } else if (sacch->getCallType() == NXDN_CALL_TYPE_INDIVIDUAL) {
            setType("individual");
        } else {
            setType("");
        }
        setSource(sacch->getSourceUnitId());
        setDestination(sacch->getDestinationId());
    }
}

void MetaCollector::reset() {
    hold();
    setSync("");
    setType("");
    setSource(0);
    setDestination(0);
    release();
}