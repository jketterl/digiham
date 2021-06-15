#include "nxdn_meta.hpp"
#include "types.hpp"

using namespace Digiham::Nxdn;

void MetaWriter::sendMetaData() {
    std::map<std::string, std::string> metadata;

    if (sync != "") {
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

    sendMetaMap(metadata);
}

std::string MetaWriter::getProtocol() {
    return "NXDN";
}

void MetaWriter::setSync(std::string sync) {
    if (this->sync == sync) return;
    this->sync = sync;
    sendMetaData();
}

void MetaWriter::setSacch(SacchSuperframe* sacch) {
    if (this->sacch != nullptr) {
        delete this->sacch;
    }
    this->sacch = sacch;
    sendMetaData();
}

void MetaWriter::reset() {
    hold();
    setSync("");
    setSacch(nullptr);
    release();
}