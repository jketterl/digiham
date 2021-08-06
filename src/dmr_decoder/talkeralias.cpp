#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <codecvt>
#include <locale>
#include "talkeralias.hpp"

#include <iostream>


using namespace Digiham::Dmr;

TalkerAliasCollector::TalkerAliasCollector():
    data((unsigned char*) malloc(sizeof(unsigned char) * 32))
{}

TalkerAliasCollector::~TalkerAliasCollector() {
    free(data);
}

void TalkerAliasCollector::reset() {
    if (headerSeen) std::cerr << "talkeralias reset\n";
    headerSeen = false;
    blocks = 0;
}

bool TalkerAliasCollector::isComplete() {
    if (!headerSeen) {
        return false;
    }

    if (dataFormat == TALKER_ALIAS_FORMAT_7BIT) {
        // unhandled
        return false;
    } else {
        std::string result = getContents();
        //std::cerr << "prelim talker alias: " << result << "; we have: " << result.length() << "; we want: " << +length << "\n";
        if (result.length() >= length) return true;
    }
    return false;
}

void TalkerAliasCollector::setHeader(unsigned char *data) {
    dataFormat = data[0] >> 6;
    length = (data[0] & 0b00111110) >> 1;

    std::cerr << "talker alias format: " << +dataFormat << "; length: " << +length << "\n";

    if (dataFormat == TALKER_ALIAS_FORMAT_7BIT) {
        // unhandled
    } else {
        std::memcpy(this->data, data + 1, 6);
    }

    headerSeen = true;
}

void TalkerAliasCollector::setBlock(int block, unsigned char *data) {
    if (!headerSeen) return;
    std::cerr << "received talker alias block " << block << "\n";
    if (dataFormat == TALKER_ALIAS_FORMAT_7BIT) {
        // unhandled
    } else {
        size_t offset = 6 + block * 7;
        std::memcpy(this->data + offset, data, 7);
    }

    blocks |= 1 << block;
}

std::string TalkerAliasCollector::getContents() {
    if (!headerSeen) {
        return "";
    }

    std::string result = "";
    if (dataFormat == TALKER_ALIAS_FORMAT_7BIT) {
        // unhandled
        return "";
    } else if (dataFormat == TALKER_ALIAS_FORMAT_UTF16) {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
        for (int i = 0; i < 4; i++) {
            unsigned char mask = (1 << i) - 1;
            // check if the required blocks are there
            if ((blocks & mask) != mask) break;
            // manual endianness conversion because i couldn't find a better way
            unsigned int chars = (6 + i * 7) / 2;
            auto econv = (char16_t*) malloc(chars * sizeof(char16_t));
            for (int k = 0; k < chars; k++) {
                econv[k] = (data[k * 2] << 8) | data[k * 2 + 1];
            }
            std::u16string s(econv, chars);
            result = converter.to_bytes(s);
            free(econv);
        }
    } else {
        for (int i = 0; i < 4; i++) {
            unsigned char mask = (1 << i) - 1;
            // check if the required blocks are there
            if ((blocks & mask) != mask) break;
            result = std::string((char*)data, 6 + i * 7);
        }
    }
    if (result.length() > length) {
        return result.substr(0, length);
    }
    return result;
}