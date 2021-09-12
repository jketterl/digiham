#include "talkeralias.hpp"
#include "charset.hpp"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <codecvt>
#include <locale>
#include <cassert>
#include <sstream>

using namespace Digiham::Dmr;

TalkerAliasCollector::TalkerAliasCollector():
    data((unsigned char*) malloc(sizeof(unsigned char) * 28))
{}

TalkerAliasCollector::~TalkerAliasCollector() {
    free(data);
}

void TalkerAliasCollector::reset() {
    blocks = 0;
}

bool TalkerAliasCollector::isComplete() {
    // cannot be complete without the header, and we cannot evaluate any data until we have that.
    if (!hasHeader()) {
        return false;
    }

    unsigned char bytes = collectedBytes();

    switch (getDataFormat()) {
        case TALKER_ALIAS_FORMAT_7BIT:
            return ((bytes * 7) / 8) - 1 >= getLength();
        case TALKER_ALIAS_FORMAT_8BIT:
            // first byte unusable
            return bytes - 1 >= getLength();
        case TALKER_ALIAS_FORMAT_UTF8: {
            // we don't know until we try
            std::string content = getContents();
            return content.length() >= getLength();
        }
        case TALKER_ALIAS_FORMAT_UTF16:
            // first byte unusable
            return (bytes - 1) / 2 >= getLength();
    }

    // if we get to here, something went wrong anyway
    return false;
}

void TalkerAliasCollector::setBlock(int block, unsigned char *data) {
    assert(block < 4);
    size_t offset = (size_t) block * 7;
    std::memcpy(this->data + offset, data, 7);

    blocks |= 1 << block;
}

std::string TalkerAliasCollector::getContents() {
    // cannot evaluate the data without a header
    if (!hasHeader()) {
        return "";
    }

    unsigned char bytes = collectedBytes();
    std::string result;

    switch (getDataFormat()) {
        case TALKER_ALIAS_FORMAT_7BIT: {
            std::stringstream ss;
            for (size_t i = 0; i < bytes; i += 7) {
                ss << convert7BitData(data + i);
            }
            // first character is trash because it was built using the header bits
            result = ss.str().substr(1);
            break;
        }
        case TALKER_ALIAS_FORMAT_8BIT:
            // first byte unusable
            result = Converter::convertToUtf8((char*) data + 1, bytes - 1);
            break;
        case TALKER_ALIAS_FORMAT_UTF8:
            // first byte unusable
            result = std::string((char*) data + 1, bytes - 1);
            break;
        case TALKER_ALIAS_FORMAT_UTF16:
            std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
            // first byte unusable
            unsigned int chars = (bytes - 1) / 2;
            auto src = data + 1;
            // manual endianness conversion because i couldn't find a better way
            auto econv = (char16_t*) malloc(chars * sizeof(char16_t));
            for (int k = 0; k < chars; k++) {
                econv[k] = (src[k * 2] << 8) | src[k * 2 + 1];
            }
            std::u16string s(econv, chars);
            result = converter.to_bytes(s);
            free(econv);
    }

    if (result.length() > getLength()) {
        result = result.substr(0, getLength());
    }

    return result;
}

bool TalkerAliasCollector::hasHeader() {
    return blocks & 1;
}

unsigned char TalkerAliasCollector::getDataFormat() {
    return data[0] >> 6;
}

unsigned char TalkerAliasCollector::getLength() {
    return (data[0] & 0b00111110) >> 1;
}

unsigned char TalkerAliasCollector::collectedBytes() const {
    int i;
    for (i = 0; i < 4; i++) {
        unsigned char mask = (1 << (i + 1)) - 1;
        // check if the required blocks are there
        if ((blocks & mask) != mask) break;
    }
    return i * 7;
}

std::string TalkerAliasCollector::convert7BitData(unsigned char *start) {
    unsigned char res[8] = { 0 };
    res[0] = (start[0] & 0b11111110) >> 1;
    res[1] = (start[0] & 0b00000001) << 6 | (start[1] & 0b11111100) >> 2;
    res[2] = (start[1] & 0b00000011) << 5 | (start[2] & 0b11111000) >> 3;
    res[3] = (start[2] & 0b00000111) << 4 | (start[3] & 0b11110000) >> 4;
    res[4] = (start[3] & 0b00001111) << 3 | (start[4] & 0b11100000) >> 5;
    res[5] = (start[4] & 0b00011111) << 2 | (start[5] & 0b11000000) >> 6;
    res[6] = (start[5] & 0b00111111) << 1 | (start[6] & 0b10000000) >> 7;
    res[7] = start[6] & 0b01111111;
    return std::string((char*) res, 8);
}