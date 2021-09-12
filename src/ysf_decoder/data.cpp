#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cstdint>

#include "data.hpp"
#include "gps.hpp"

extern "C" {
#include "radio_types.h"
}

using namespace Digiham::Ysf;

DataFrame::DataFrame(unsigned char *data):
    data((unsigned char*) malloc(sizeof(unsigned char) * 20))
{
    std::memcpy(this->data, data, 20);
}

DataFrame::~DataFrame() {
    free(data);
}

uint32_t DataFrame::getCommand() {
    return data[1] << 16 | data[2] << 8 | data[3];
}

Digiham::Coordinate* DataFrame::getGpsCoordinate() {
    uint32_t command = getCommand();
    if (command != COMMAND_SHORT_GPS) {
        return nullptr;
    }

    return Gps::parse(data + 5);
}

std::string DataFrame::getRadio() {
    uint8_t radio_id = data[4];
    return get_radio_type(radio_id);
}

DataCollector::DataCollector():
    // 20 bytes is only sufficient for V/D mode 2
    // if this is to be used in other modes, this needs to be increased
    data((unsigned char*) malloc(sizeof(unsigned char) * 20))
{}

DataCollector::~DataCollector() {
    free(data);
}

void DataCollector::reset() {
    nextOffset = 0;
}

void DataCollector::collect(unsigned char *data, unsigned char offset) {
    // see constructor
    assert(offset < 2);
    if (offset != nextOffset) {
        // sequence error; reset
        nextOffset = 0;
        return;
    } else {
        nextOffset = offset + 1;
    }

    std::memcpy(this->data + offset * 10, data, 10);
}

bool DataCollector::hasCollected(unsigned char num) {
    return nextOffset >= num;
}

DataFrame* DataCollector::getDataFrame() {
    if (data[18] != 0x03) {
        return nullptr;
    }

    uint8_t checksum = 0;
    for (int i = 0; i < 19; i++) checksum += data[i];

    if (checksum != data[19]) {
        return nullptr;
    }

    return new DataFrame(data);
}