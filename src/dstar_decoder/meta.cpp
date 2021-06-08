#include "meta.hpp"
#include <sstream>

#include <iostream>

using namespace Digiham::DStar;

MetaWriter::MetaWriter(FILE* out): file(out) {}

MetaWriter::MetaWriter(): MetaWriter(nullptr) {}

MetaWriter::~MetaWriter() {
    fclose(file);
}

void MetaWriter::setFile(FILE* out) {
    file = out;
}

void MetaWriter::setSync(std::string sync) {
    if (sync == this->sync) return;
    this->sync = sync;
    sendMetaData();
}

void MetaWriter::setHeader(Header* header) {
    if (this->header != nullptr) {
        delete(this->header);
    }
    this->header = header;
    if (header != nullptr) {
        if (header->isVoice()) {
            setSync("voice");
        } else {
            setSync("data");
        }
    } else {
        setSync("");
    }
    sendMetaData();
}

void MetaWriter::setMessage(std::string message) {
    if (message == this->message) return;
    this->message = message;
    sendMetaData();
}

void MetaWriter::setDPRS(std::string dprs) {
    if (dprs == this->dprs) return;
    this->dprs = dprs;
    sendMetaData();
}

void MetaWriter::reset() {
    hold();
    setHeader(nullptr);
    setSync("");
    setMessage("");
    setDPRS("");
    release();
}

void MetaWriter::hold() {
    held = true;
}

void MetaWriter::release() {
    held = false;
    sendMetaData();
}

void MetaWriter::sendMetaData() {
    if (/*file == nullptr || */held) {
        return;
    }

    std::map<std::string, std::string> metadata;

    if (sync != "") {
        metadata["sync"] = sync;
    }

    if (header != nullptr) {
        metadata["departure"] = header->getDepartureRepeater();
        metadata["destination"] = header->getDestinationRepeater();
        metadata["ourcall"] = header->getOwnCallsign();
        metadata["yourcall"] = header->getCompanion();
    }

    if (message != "") {
        metadata["message"] = message;
    }

    if (dprs != "") {
        metadata["dprs"] = dprs;
    }

    std::stringstream ss;
    ss << "mode:dstar";
    for (std::map<std::string, std::string>::iterator it = metadata.begin(); it != metadata.end(); it++) {
        ss << ";" << it->first << ":" << it->second;
    }
    ss << "\n";

    std::string metaString = ss.str();
    std::cerr << "metadata: " << metaString;
    if (file == nullptr) return;
    fwrite(metaString.c_str(), 1, metaString.length(), file);
    fflush(file);
}