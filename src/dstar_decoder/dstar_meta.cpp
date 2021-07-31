#include "dstar_meta.hpp"

using namespace Digiham::DStar;

void MetaCollector::setSync(std::string sync) {
    if (sync == this->sync) return;
    this->sync = sync;
    sendMetaData();
}

void MetaCollector::setHeader(Header* header) {
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

void MetaCollector::setMessage(std::string message) {
    if (message == this->message) return;
    this->message = message;
    sendMetaData();
}

void MetaCollector::setDPRS(std::string dprs) {
    if (dprs == this->dprs) return;
    this->dprs = dprs;
    sendMetaData();
}

void MetaCollector::setGPS(float lat, float lon) {
    if (gpsSet && this->lat == lat && this->lon == lon) return;
    this->lat = lat;
    this->lon = lon;
    gpsSet = true;
}

void MetaCollector::reset() {
    hold();
    setHeader(nullptr);
    setSync("");
    setMessage("");
    setDPRS("");
    setGPS(0, 0);
    gpsSet = false;
    release();
}

std::string MetaCollector::getProtocol() {
    return "DSTAR";
}

std::map<std::string, std::string> MetaCollector::collect() {
    auto metadata = Digiham::MetaCollector::collect();

    if (!sync.empty()) {
        metadata["sync"] = sync;
    }

    if (header != nullptr) {
        metadata["departure"] = header->getDepartureRepeater();
        metadata["destination"] = header->getDestinationRepeater();
        metadata["ourcall"] = header->getOwnCallsign();
        metadata["yourcall"] = header->getCompanion();
    }

    if (!message.empty()) {
        metadata["message"] = message;
    }

    if (!dprs.empty()) {
        metadata["dprs"] = dprs;
    }

    if (gpsSet) {
        metadata["lat"] = std::to_string(lat);
        metadata["lon"] = std::to_string(lon);
    }

    return metadata;
}