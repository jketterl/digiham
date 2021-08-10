#include "ysf_meta.hpp"

using namespace Digiham::Ysf;

MetaCollector::~MetaCollector() {
    delete coord;
}

std::string MetaCollector::getProtocol() {
    return "YSF";
}

std::map<std::string, std::string> MetaCollector::collect() {
    auto result = Digiham::MetaCollector::collect();

    if (!mode.empty()) {
        result["mode"] = mode;
    }

    if (!destination.empty()) {
        result["target"] = destination;
    }

    if (!source.empty()) {
        result["source"] = source;
    }

    if (!up.empty()) {
        result["up"] = up;
    }

    if (!down.empty()) {
        result["down"] = down;
    }

    if (!radio.empty()) {
        result["radio"] = radio;
    }

    if (coord != nullptr) {
        result["lat"] = std::to_string(coord->lat);
        result["lon"] = std::to_string(coord->lon);
    }

    return result;
}

void MetaCollector::reset () {
    hold();
    setMode("");
    setDestination("");
    setSource("");
    setUp("");
    setDown("");
    setRadio("");
    setGps(nullptr);
    release();
}

void MetaCollector::setMode(std::string mode) {
    if (this->mode == mode) return;
    this->mode = mode;
    sendMetaData();
}

void MetaCollector::setDestination(std::string destination) {
    if (this->destination == destination) return;
    this->destination = destination;
    sendMetaData();
}

void MetaCollector::setSource(std::string source) {
    if (this->source == source) return;
    this->source = source;
    sendMetaData();
}

void MetaCollector::setUp(std::string up) {
    if (this->up == up) return;
    this->up = up;
    sendMetaData();
}

void MetaCollector::setDown(std::string down) {
    if (this->down == down) return;
    this->down = down;
    sendMetaData();
}

void MetaCollector::setRadio(std::string radio) {
    if (this->radio == radio) return;
    this->radio = radio;
    sendMetaData();
}

void MetaCollector::setGps(coordinate* coord) {
    // TODO implement actual comparison
    if (this->coord == coord) return;
    // prevent race conditions
    auto old = this->coord;
    this->coord = coord;
    delete old;
    sendMetaData();
}