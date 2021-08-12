#include "dstar_meta.hpp"

using namespace Digiham::DStar;

void MetaCollector::setSync(std::string sync) {
    if (sync == this->sync) return;
    this->sync = sync;
    sendMetaData();
}

void MetaCollector::setFromHeader(Header* header) {
    hold();
    if (header->isVoice()) {
        setSync("voice");
    } else {
        setSync("data");
    }
    setDeparture(header->getDepartureRepeater());
    setDestination(header->getDestinationRepeater());
    setOurCall(header->getOwnCallsign());
    setYourCall(header->getCompanion());
    release();
}

void MetaCollector::setMessage(std::string message) {
    if (message == this->message) return;
    this->message = message;
    sendMetaData();
}

void MetaCollector::setDeparture(std::string departure) {
    if (departure == this->departure) return;
    this->departure = departure;
    sendMetaData();
}

void MetaCollector::setDestination(std::string destination) {
    if (destination == this->destination) return;
    this->destination = destination;
    sendMetaData();
}

void MetaCollector::setOurCall(std::string ourCall) {
    if (ourCall == this->ourCall) return;
    this->ourCall = ourCall;
    sendMetaData();
}

void MetaCollector::setYourCall(std::string yourCall) {
    if (yourCall == this->yourCall) return;
    this->yourCall = yourCall;
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
    setSync("");
    setMessage("");
    setDeparture("");
    setDestination("");
    setOurCall("");
    setYourCall("");
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

    if (!departure.empty()) {
        metadata["departure"] = departure;
    }

    if (!destination.empty()) {
        metadata["destination"] = destination;
    }

    if (!ourCall.empty()) {
        metadata["ourcall"] = ourCall;
    }

    if (!yourCall.empty()) {
        metadata["yourcall"] = yourCall;
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