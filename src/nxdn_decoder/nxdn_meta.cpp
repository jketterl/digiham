#include "nxdn_meta.hpp"
#include "types.hpp"

using namespace Digiham::Nxdn;

std::map<std::string, std::string> MetaCollector::collect() {
    auto metadata = Digiham::MetaCollector::collect();

    if (!sync.empty()) {
        metadata["sync"] = sync;
    }

    if (sacch != nullptr) {
        if (sacch->getMessageType() == NXDN_MESSAGE_TYPE_VCALL) {
            if (sacch->getCallType() == NXDN_CALL_TYPE_CONFERENCE) {
                metadata["type"] = "conference";
            } else if (sacch->getCallType() == NXDN_CALL_TYPE_INDIVIDUAL) {
                metadata["type"] = "individual";
            }
            metadata["source"] = std::to_string(sacch->getSourceUnitId());
            metadata["destination"] = std::to_string(sacch->getDestinationId());
        }
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

void MetaCollector::setSacch(SacchSuperframe* sacch) {
    delete this->sacch;
    this->sacch = sacch;
    sendMetaData();
}

void MetaCollector::reset() {
    hold();
    setSync("");
    setSacch(nullptr);
    release();
}