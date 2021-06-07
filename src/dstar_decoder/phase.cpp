#include "phase.hpp"
#include "crc.hpp"
#include <iostream>
#include <cstring>
#include <sstream>

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::DStar;

Phase* SyncPhase::process(Ringbuffer* data, size_t& read_pos) {
    while (data->available(read_pos) > SYNC_SIZE) {
        //std::cerr << "scanning ringbuffer at " << read_pos << "\n";

        uint8_t potential_sync[SYNC_SIZE];
        data->read((char*) potential_sync, read_pos, SYNC_SIZE);

        if (hamming_distance(potential_sync, (uint8_t*) header_sync, SYNC_SIZE) <= 2) {
            data->advance(read_pos, SYNC_SIZE);
            return new HeaderPhase();
        }

        if (hamming_distance(potential_sync, (uint8_t*) voice_sync, SYNC_SIZE) <= 1) {
            std::cerr << "found a voice sync at pos " << read_pos << "\n";
            data->advance(read_pos, SYNC_SIZE);
            return new VoicePhase(0);
        }

        data->advance(read_pos, 1);
    }
    return nullptr;
}

Phase* HeaderPhase::process(Ringbuffer* data, size_t& read_pos) {
    unsigned char* raw = (unsigned char*) malloc(sizeof(unsigned char) * Header::bits);
    data->read((char*) raw, read_pos, Header::bits);

    Header* header = Header::parse(raw);
    free(raw);
    if (header == nullptr) {
        data->advance(read_pos, 1);
        return new SyncPhase();
    }

    if (!header->isCrcValid()) {
        delete header;
        data->advance(read_pos, 1);
        return new SyncPhase();
    }

    std::cerr << "found header: " << header->toString() << "\n";

    data->advance(read_pos, Header::bits);

    if (header->isVoice()) {
        delete(header);
        return new VoicePhase();
    }

    delete(header);
    return new SyncPhase();
}

VoicePhase::VoicePhase(int frameCount): Phase(), frameCount(frameCount) {
    scrambler = new Scrambler();
}

// default set up is with a sync due immediately, which is what we'd expect after a header
// when starting after a voice sync, pass frameCount = 0 to the constructor above
VoicePhase::VoicePhase(): VoicePhase(21) {}

VoicePhase::~VoicePhase() {
    delete scrambler;
}

Phase* VoicePhase::process(Ringbuffer* data, size_t& read_pos) {
    char* voice = (char*) malloc(sizeof(char) * 72);
    data->read(voice, read_pos, 72);
    data->advance(read_pos, 72);

    char* voice_packed = (char*) malloc(sizeof(char) * 9);
    memset(voice_packed, 0, 9);
    for (int i = 0; i < 72; i++) {
        voice_packed[i / 8] |= (voice[i] & 1) << ( i % 8 );
    }
    free(voice);

    fwrite(voice_packed, sizeof(char), 9, stdout);
    free(voice_packed);


    // only 24 bits of data, but the terminator is 48 bits long, so we have to look ahead
    uint8_t* data_frame = (uint8_t*) malloc(sizeof(uint8_t) * 48);
    data->read((char*) data_frame, read_pos, 48);
    data->advance(read_pos, 24);

    if (hamming_distance(data_frame, (uint8_t*) terminator, TERMINATOR_SIZE) == 0) {
        std::cerr << "terminator received\n";
        // move another 24 since it's clear that this is all used up now
        data->advance(read_pos, 24);
        return new SyncPhase();
    }
    if (isSyncDue()) {
        if (hamming_distance(data_frame, (uint8_t*) voice_sync, SYNC_SIZE) > 1) {
            if (syncMissing++ > 2) {
                std::cerr << "too many missed syncs, ending voice mode\n";
                return new SyncPhase();
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

    free(data_frame);

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
        std::cerr << "parsed message: \"" << message << "\"\n";
    }
    if (headerCount == 41) {
        Header* h = new Header(header);
        if (h->isCrcValid()) {
            std::cerr << "inline header: " << h->toString() << "\n";
        }
    }
    size_t pos = simpleData.find("\r");
    if (pos != std::string::npos) {
        std::string something = simpleData.substr(0, pos + 1);
        if (something.length() >= 10 && something.substr(0, 5) == "$$CRC" && something.at(9) == ',') {
            std::stringstream ss;
            uint16_t checksum;
            ss << std::hex << something.substr(5, 4);
            ss >> checksum;

            bool valid = Crc::isCrcValid((unsigned char*) something.substr(10).c_str(), something.length() - 10, checksum);
            if (valid) {
                std::cerr << "parsed DPRS: " << something.substr(10, something.length() - 11) << "\n";
            }
        } else {
            std::cerr << "parsed simple data: " << something << "\n";
        }
        simpleData = simpleData.substr(pos + 1);
    }
}