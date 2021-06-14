#include "dstar_meta.hpp"
#include <sstream>

using namespace Digiham::DStar;

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

void MetaWriter::setGPS(float lat, float lon) {
    if (gpsSet && this->lat == lat && this->lon == lon) return;
    this->lat = lat;
    this->lon = lon;
    gpsSet = true;
}

void MetaWriter::reset() {
    hold();
    setHeader(nullptr);
    setSync("");
    setMessage("");
    setDPRS("");
    setGPS(0, 0);
    gpsSet = false;
    release();
}

std::string MetaWriter::getProtocol() {
    return "DSTAR";
}

void MetaWriter::sendMetaData() {
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

    if (gpsSet) {
        metadata["lat"] = std::to_string(lat);
        metadata["lon"] = std::to_string(lon);
    }

    sendMetaMap(metadata);
}