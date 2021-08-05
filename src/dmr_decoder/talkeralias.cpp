#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "talkeralias.hpp"

#include <iostream>

using namespace Digiham::Dmr;

TalkerAliasCollector::TalkerAliasCollector():
    data((unsigned char*) malloc(sizeof(unsigned char) * 31))
{}

TalkerAliasCollector::~TalkerAliasCollector() {
    free(data);
}

void TalkerAliasCollector::reset() {
    checked = 0;
}

bool TalkerAliasCollector::isComplete() {
    if (!(checked & 0b1000)) {
        return false;
    }
    // TODO implement other data formats
    if (dataFormat != TALKER_ALIAS_FORMAT_8BIT && dataFormat && TALKER_ALIAS_FORMAT_UTF8) return false;

    unsigned char required = 0b1000;
    if (length > 6) {
        required = 0b1001;
    } else if (length > 13) {
        required = 0b1011;
    } else if (length > 20) {
        required = 0b1111;
    }
    return checked == required;
}

void TalkerAliasCollector::setHeader(unsigned char *data) {
    dataFormat = data[0] >> 6;
    length = (data[0] & 0b00111110) >> 1;

    std::cerr << "talker alias format: " << +dataFormat << "; length: " << +length << "\n";

    if (dataFormat == TALKER_ALIAS_FORMAT_8BIT || dataFormat == TALKER_ALIAS_FORMAT_UTF8) {
        std::memcpy(this->data, data + 1, std::min(length, (unsigned char) 6));
    }

    checked |= 0b1000;
}

void TalkerAliasCollector::setBlock(int block, unsigned char *data) {
    if (dataFormat == TALKER_ALIAS_FORMAT_8BIT || dataFormat == TALKER_ALIAS_FORMAT_UTF8) {
        size_t offset = 6 + block * 7;
        std::memcpy(this->data + offset, data, std::min(length - offset, (size_t) 7));
    }
    checked |= 1 << block;
}

std::string TalkerAliasCollector::getContents() {
    return std::string((char*) data, length);
}