#include "dstar_phase.hpp"
#include "crc.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::DStar;

Digiham::Phase* SyncPhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    //std::cerr << "scanning ringbuffer at " << read_pos << "\n";

    uint8_t* potential_sync = data->getReadPointer();

    if (hamming_distance(potential_sync, (uint8_t*) header_sync, SYNC_SIZE) <= 2) {
        data->advance(SYNC_SIZE);
        return new HeaderPhase();
    }

    if (hamming_distance(potential_sync, (uint8_t*) voice_sync, SYNC_SIZE) <= 1) {
        std::cerr << "found a voice sync\n";
        data->advance(SYNC_SIZE);
        return new VoicePhase(0);
    }

    // as long as we don't find any sync, move ahead, bit by bit
    data->advance(1);
    // tell decoder that we'll continue
    return this;
}

Digiham::Phase* HeaderPhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    unsigned char* raw = data->getReadPointer();

    Header* header = Header::parse(raw);
    if (header == nullptr) {
        data->advance(1);
        return new SyncPhase();
    }

    if (!header->isCrcValid()) {
        delete header;
        data->advance(1);
        return new SyncPhase();
    }

    std::cerr << "found header: " << header->toString() << "\n";

    data->advance(Header::bits);

    if (header->isVoice()) {
        // only set the header when we're actually entering voice phase
        // since data phase is not implemented and we don't detect it's end
        ((MetaWriter*) meta)->setHeader(header);
        return new VoicePhase();
    }

    delete(header);
    return new SyncPhase();
}

VoicePhase::VoicePhase(int frameCount): Phase(), frameCount(frameCount), syncCount(0) {
    scrambler = new Scrambler();
}

// default set up is with a sync due immediately, which is what we'd expect after a header
// when starting after a voice sync, pass frameCount = 0 to the constructor above
VoicePhase::VoicePhase(): VoicePhase(21) {
    // header already counts as one sync, so we can get started right away
    syncCount = 1;
}

VoicePhase::~VoicePhase() {
    delete scrambler;
}

Digiham::Phase* VoicePhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    // only output actual voice frames if we are confident about the sync
    if (syncCount >= 1) {
        unsigned char* voice = data->getReadPointer();

        unsigned char* voice_packed = output->getWritePointer();
        std::memset(voice_packed, 0, 9);
        for (int i = 0; i < 72; i++) {
            voice_packed[i / 8] |= (voice[i] & 1) << ( i % 8 );
        }
        output->advance(9);
    }
    data->advance(72);

    uint8_t* data_frame = data->getReadPointer();
    data->advance(24);

    if (
        hamming_distance(data_frame, (uint8_t*) terminator, TERMINATOR_SIZE) <= 1 ||
        // some repeaters only send the second half in the data frame
        hamming_distance(data_frame, ((uint8_t*) terminator) + 24, TERMINATOR_SIZE - 24) <= 1
    ) {
        // move another 24 since it's clear that this is all used up now
        std::cerr << "terminator frame received, ending voice mode\n";
        data->advance(24);
        ((MetaWriter*) meta)->reset();
        return new SyncPhase();
    }
    if (isSyncDue()) {
        if (hamming_distance(data_frame, (uint8_t*) voice_sync, SYNC_SIZE) > 1) {
            if (--syncCount < 0) {
                std::cerr << "too many missed syncs, ending voice mode\n";
                ((MetaWriter*) meta)->reset();
                return new SyncPhase();
            }
        } else {
            // increase sync count, cap at 3
            if (++syncCount > 3) syncCount = 3;
            if (syncCount > 1) {
                ((MetaWriter*) meta)->setSync("voice");
            }
        }
        parseFrameData();
        resetFrames();
    } else {
        scrambler->reset();
        unsigned char* data_descrambled = (unsigned char*) malloc(sizeof(unsigned char) * 24);
        scrambler->scramble(data_frame, data_descrambled, 24);

        unsigned char* data_bytes = (unsigned char*) malloc(sizeof(char) * 3);
        memset(data_bytes, 0, sizeof(unsigned char) * 3);
        for (int i = 0; i < 24; i++) {
            data_bytes[i / 8] |= data_descrambled[i] << ( i % 8 );
        }
        free(data_descrambled);
        collectDataFrame(data_bytes);
        free(data_bytes);

        frameCount++;
    }

    return this;
}

bool VoicePhase::isSyncDue() {
    return frameCount >= 20;
}

void VoicePhase::resetFrames() {
    frameCount = 0;
    memset(message, 0, 20);
    messageBlocks = 0;
    memset(header, 0, 41);
    headerCount = 0;
}

void VoicePhase::collectDataFrame(unsigned char* data) {
    memcpy(collected_data + (frameCount % 2) * 3, data, 3);
    if (frameCount % 2 == 0) {
        return;
    }

    switch (collected_data[0] >> 4) {
        case 0x04: {
            // 20-character d-star message
            int block = collected_data[0] & 0x0F;
            if (block > 3) break;
            memcpy(message + block * 5, collected_data + 1, 5);
            messageBlocks |= 1 << block;
            break;
        }
        case 0x05: {
            // inline header data
            int bytes = collected_data[0] & 0x0F;
            if (bytes > 5) break;
            if (headerCount + bytes > 41) break;
            memcpy(header + headerCount, collected_data + 1, bytes);
            headerCount += bytes;
            break;
        }
        case 0x03: {
            int bytes = collected_data[0] & 0x0F;
            if (bytes > 5) break;
            simpleData += std::string((char*) collected_data + 1, bytes);
            break;
        }
        case 0x00:
        case 0x01:
        case 0x02:
        // 0x66 is an explicitly empty frame... doesn't make a difference here
        case 0x06:
        case 0x07:
        case 0x0A:
        case 0x0B:
        case 0x0D:
        case 0x0E:
        case 0x0F:
            // reserved. ignore.
            break;
        default:
            std::cerr << "received unknown data (mini header = " << std::hex << +collected_data[0] << ")\n";
    }
}

void VoicePhase::parseFrameData() {
    if (messageBlocks == 0x0F) {
        ((MetaWriter*) meta)->setMessage(std::string((char*)message, 20));
    }
    if (headerCount == 41) {
        unsigned char* headerData = (unsigned char*) malloc(41);
        memcpy(headerData, header, 41);
        Header* h = new Header(headerData);
        if (h->isCrcValid()) {
            ((MetaWriter*) meta)->setHeader(h);
        } else {
            delete(h);
        }
    }
    size_t pos;
    while ((pos = simpleData.find('\r')) != std::string::npos) {
        std::string something = simpleData.substr(0, pos + 1);
        if (something.length() >= 10 && something.substr(0, 5) == "$$CRC" && something.at(9) == ',') {
            std::stringstream ss;
            uint16_t checksum;
            ss << std::hex << something.substr(5, 4);
            ss >> checksum;

            bool valid = Crc::isCrcValid((unsigned char*) something.substr(10).c_str(), something.length() - 10, checksum);
            if (valid) {
                ((MetaWriter*) meta)->setDPRS(something.substr(10, something.length() - 11));
            }
        } else if (something.length() > 5 && something.at(0) == '$') {
            parseNMEAData(something);
        } else {
            std::cerr << "parsed simple data: " << something << "\n";
        }
        // termination may be \r or \r\n - anticipate both
        simpleData = simpleData.substr(pos + 1 + (simpleData.length() > pos + 1 && simpleData.at(pos + 1) == '\n'));
    }
}

void VoicePhase::parseNMEAData(const std::string& input) {
    size_t checksum_pos = input.find_last_of("*");
    if (checksum_pos == std::string::npos) {
        std::cerr << "no NMEA checksum available\n";
        return;
    }
    if (checksum_pos + 2 > input.length()) {
        std::cerr << "NMEA checksum incomplete\n";
        return;
    }

    std::string body = input.substr(1, checksum_pos - 1);
    // std::string talker = body.substr(0, 2);
    std::string message = body.substr(2, 3);
    uint8_t checksum = 0;
    for (int i = 0; i < body.length(); i++) {
        checksum ^= body.at(i);
    }

    std::stringstream ss;
    uint16_t to_check;
    ss << std::hex << input.substr(checksum_pos + 1, 2);
    ss >> to_check;

    if (checksum != to_check) {
        std::cerr << "NMEA checksum failure\n";
        return;
    }

    std::vector<std::string> fields;
    std::stringstream splitter(body);
    std::string item;
    while (getline(splitter, item, ',')) {
        fields.push_back(item);
    }

    if (message == "GGA") {
        float lat_combined = std::stof(fields[2]);
        float lat = (int) lat_combined / 100;
        lat += (lat_combined - lat * 100) / 60;
        if (fields[3] == "S") lat *= -1;
        float lon_combined = std::stof(fields[4]);
        float lon = (int) lon_combined / 100;
        lon += (lon_combined - lon * 100) / 60;
        if (fields[5] == "W") lon *= -1;

        ((MetaWriter*) meta)->setGPS(lat, lon);
    }
}