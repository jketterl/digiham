#include "ysf_phase.hpp"
#include "ysf_meta.hpp"

#include <cstring>

extern "C" {
#include "hamming_distance.h"
#include "whitening.h"
#include "crc16.h"
#include "trellis.h"
}

using namespace Digiham::Ysf;

bool Phase::isSync(unsigned char *input) {
    return hamming_distance((uint8_t*) input, (uint8_t*) ysf_sync, SYNC_SIZE) <= 3;
}

int SyncPhase::getRequiredData() {
    return SYNC_SIZE;
}


Digiham::Phase* SyncPhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    if (isSync(data->getReadPointer())) {
        return new FramePhase();
    }

    // as long as we don't find any sync, move ahead, symbol by symbol
    data->advance(1);
    // tell decoder that we'll continue
    return this;
}

FramePhase::~FramePhase() {
    delete runningFich;
    delete dataCollector;
}

int FramePhase::getRequiredData() {
    return FRAME_SIZE;
}

Digiham::Phase* FramePhase::process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) {
    if (isSync(data->getReadPointer())) {
        if (++syncCount > 12) syncCount = 12;
    } else {
        if (--syncCount < 0) {
            ((MetaCollector*) meta)->reset();
            return new SyncPhase();
        }
    }

    data->advance(SYNC_SIZE);

    Fich* fich = Fich::parse(data->getReadPointer());
    if (fich != nullptr) {
        delete runningFich;
        runningFich = fich;
    }

    data->advance(FICH_SIZE);

    unsigned char* payload = data->getReadPointer();

    if (runningFich != nullptr) {
        switch(runningFich->getFrameType()) {
            case FRAME_TYPE_COMMUNICATION_CHANNEL: {
                switch (runningFich->getDataType()) {
                    case DATA_TYPE_VD_TYPE_1: {
                        // V/D mode type 1
                        ((MetaCollector*) meta)->setMode("V1");
                        // contains 5 voice channel blocks à 72 bits
                        for (int i = 0; i < 5; i++) {
                            // voice decoder mode byte
                            *(output->getWritePointer()) = runningFich->getDataType();
                            // 36 dibits data channel + block offset
                            int offset = 36 + i * 72;
                            decodeV1VoicePayload(payload + offset, output->getWritePointer() + 1);
                            output->advance(10);
                        }
                        break;
                    }
                    case DATA_TYPE_VD_TYPE_2: {
                        // V/D mode type 2
                        ((MetaCollector*) meta)->setMode("DN");
                        // contains 5 voice channel blocks à 72 (data) + 32 (check) bits
                        for (int i = 0; i < 5; i++) {
                            // voice decoder mode byte
                            *(output->getWritePointer()) = runningFich->getDataType();
                            // 20 dibits data channel + block offset
                            int offset = 20 + i * 72;
                            decodeV2VoicePayload(payload + offset, output->getWritePointer() + 1);
                            output->advance(8);
                        }
                        // contains 5 data channel blocks à 40 bits
                        unsigned char dch_raw[25] = { 0 };

                        // 20 by 5 dibit matrix interleaving
                        // needs information from the FICH, so no point in doing anything if we don't have that
                        if (fich != nullptr) {
                            for (int i = 0; i < 100; i++) {
                                int inpos = ((i % 5) * 72 + (i * 2) / 10);
                                dch_raw[i / 4] |= (payload[inpos] & 3) << (6 - 2 * (i % 4));
                            }
                            decodeV2DataChannel(dch_raw, fich->getFrameNumber());
                        }
                        break;
                    }
                    case DATA_TYPE_VOICE_FR: {
                        ((MetaCollector*) meta)->setMode("VW");
                        // TODO sub header
                        int start_frame = 0;
                        if (expectSubFrame) start_frame = 3;
                        expectSubFrame = false;

                        // contains 5 voice channel blocks à 144 bits
                        for (int i = start_frame; i < 5; i++) {
                            // voice decoder mode byte
                            *(output->getWritePointer()) = runningFich->getDataType();

                            int offset = i * 72;
                            decodeFRVoicePayload(payload + offset, output->getWritePointer() + 1);

                            output->advance(19);
                        }

                        break;
                    }
                    case DATA_TYPE_DATA_FR: {
                        // not implemented
                        ((MetaCollector*) meta)->setMode("FR data");
                        break;
                    }
                }
                break;
            }
            case FRAME_TYPE_HEADER_CHANNEL: {
                auto c = (MetaCollector*) meta;
                c->reset();
                c->hold();
                unsigned char* dch = decodeHeaderDataChannel(data->getReadPointer());
                if (dch != nullptr) {
                    // CSD1 - contains Dest and Src fields
                    c->setDestination(treatYsfString(std::string((char*) dch, 10)));
                    c->setSource(treatYsfString(std::string((char*) dch + 10, 10)));
                    free(dch);
                }
                dch = decodeHeaderDataChannel(data->getReadPointer() + 36);
                if (dch != nullptr) {
                    c->setDown(treatYsfString(std::string((char*) dch, 10)));
                    c->setUp(treatYsfString(std::string((char*) dch + 10, 10)));
                    free(dch);
                }
                c->release();

                // TODO only set this on FR mode
                expectSubFrame = true;
                break;
            }
            case FRAME_TYPE_TERMINATOR_CHANNEL: {
                ((MetaCollector*) meta)->reset();
            }
        }
    }

    data->advance(PAYLOAD_SIZE);

    // tell decoder that we'll continue
    return this;
}

void FramePhase::decodeV1VoicePayload(unsigned char* in, unsigned char *out) {
    for (int k = 0; k < 36; k++) {
        out[k / 4] = (in[k] & 3) << (6 - 2 * (k % 4));
    }
}

void FramePhase::decodeV2VoicePayload(unsigned char* in, unsigned char* out) {
    uint8_t voice_interleaved[13] = { 0 };
    for (int k = 0; k < 52; k++) {
        voice_interleaved[k / 4] |= (in[k] & 3) << (6 - 2 * (k % 4));
    }

    uint8_t voice_whitened[13] = {0};
    // 26 by 4 deinterleave
    for (int k = 0; k < 104; k++) {
        int offset = (k * 4) % 104 + k * 4 / 104;
        uint8_t inpos = offset / 8;
        uint8_t inshift = 7 - offset % 8;

        uint8_t outpos = k / 8;
        uint8_t outshift = 7 - k % 8;

        voice_whitened[outpos] |= ((voice_interleaved[inpos] >> inshift) & 1) << outshift;
    }

    uint8_t voice_tribit[13] = { 0 };
    decode_whitening(&voice_whitened[0], &voice_tribit[0], 104);

    uint8_t voice[7] = { 0 };
    decodeTribits(&voice_tribit[0], &voice[0], 27);

    // bitwise copying with offset...
    for (int k = 0; k < 22; k++) {
        int inbit_pos = k + 81;
        int inpos = inbit_pos / 8;
        int inshift = 7 - (inbit_pos % 8);

        int outbit_pos = k + 27;
        int outpos = outbit_pos / 8;
        int outshift = 7 - (outbit_pos % 8);

        voice[outpos] |= ((voice_tribit[inpos] >> inshift) & 1) << outshift;
    }

    interleaveV2VoicePayload(voice, out);
}

void FramePhase::decodeTribits(uint8_t* input, uint8_t* output, uint8_t num) {
    int i, k;
    std::memset(output, 0, (num + 7) / 8);
    for (i = 0; i < num; i++) {
        int offset = i * 3;
        uint8_t tribit = 0;
        for (k = 0; k < 3; k++) {
            int pos = (offset + k) / 8;
            int shift = 7 - ((offset + k) % 8);

            tribit = (tribit << 1) | ((input[pos] >> shift) & 1);
        }

        int outpos = i / 8;
        int outshift = 7 - (i % 8);

        output[outpos] |= tribit_majority_table[tribit] << outshift;
    }
}

void FramePhase::interleaveV2VoicePayload(uint8_t payload[7], uint8_t result[7]) {
    for (int i = 0; i < 8; i++) result[i] = 0;

    for (int input_bit = 0; input_bit < 49; input_bit++) {
        int input_position = input_bit / 8;
        int input_shift = 7 - (input_bit % 8);

        int output_bit = v2_voice_mapping[input_bit];
        int output_position = output_bit / 8;
        int output_shift = 7 - (output_bit % 8);

        uint8_t x = (payload[input_position] >> input_shift) & 1;

        result[output_position] |= x << output_shift;
    }
}

void FramePhase::decodeV2DataChannel(unsigned char *in, unsigned char frameNumber) {
    uint8_t dch_whitened[13] = { 0 };
    uint8_t r = decode_trellis(in, 100, dch_whitened);

    uint16_t checksum = dch_whitened[10] << 8 | dch_whitened[11];
    if (!crc16((uint8_t*) &dch_whitened, 10, &checksum)) {
        return;
    }
    uint8_t dch[13] = { 0 };
    decode_whitening(dch_whitened, dch, 100);

    if (frameNumber < 6) {
        std::string contents((char*) dch, 10);
        auto collector = (MetaCollector*) meta;
        switch (frameNumber) {
            case 0:
                collector->setDestination(treatYsfString(contents));
                break;
            case 1:
                collector->setSource(treatYsfString(contents));
                break;
            case 2:
                collector->setDown(treatYsfString(contents));
                break;
            case 3:
                collector->setUp(treatYsfString(contents));
                break;
            default:
                // TODO: 4 contains REM1 & REM2
                // TODO: 5 contains REM3 & REM4
                break;
        }
        dataCollector->reset();
    }

    // frame number should not go above 7, but you never know
    if (frameNumber >= 6 && frameNumber < 8) {
        dataCollector->collect(dch, frameNumber - 6);
    }

    // we need 20 bytes for DT1 and DT2
    if (dataCollector->hasCollected(2)) {
        DataFrame* data = dataCollector->getDataFrame();
        if (data != nullptr) {
            coordinate* coord = data->getGpsCoordinate();
            ((MetaCollector*) meta)->setGps(coord);
            delete data;
        }
    }
}

void FramePhase::decodeFRVoicePayload(unsigned char *in, unsigned char *out) {
    std::memset(out, 0, 18);
    // 20 dibits sync + 100 dibits fich + block offset
    for (int k = 0; k < 72; k++) {
        out[k / 4] |= (in[k] & 3) << (6 - 2 * (k % 4));
    }

}

unsigned char *FramePhase::decodeHeaderDataChannel(unsigned char *in) {
    // contains 5 data channel blocks à 40 bits
    unsigned char dch_raw[45] = { 0 };

    // 20 by 9 dibit matrix interleaving
    // also pulls out the bits from their 72bit blocks
    for (int i = 0; i < 180; i++) {
        int pos = i / 4;
        int shift = 6 - 2 * (i % 4);

        int streampos = (i % 9) * 20 + i / 9;
        int blockpos = (streampos / 36) * 72;
        int blockoffset = streampos % 36;
        int inpos = blockpos + blockoffset;

        dch_raw[pos] |= (in[inpos] & 3) << shift;
    }

    uint8_t dch_whitened[23] = { 0 };
    uint8_t r = decode_trellis(dch_raw, 180, dch_whitened);

    uint16_t checksum = (dch_whitened[20] << 8) | dch_whitened[21];
    bool crc_valid = crc16((uint8_t *) &dch_whitened, 20, &checksum);

    if (!crc_valid) {
        return nullptr;
    }

    auto dch = (unsigned char*) malloc(sizeof(unsigned char) * 20);
    decode_whitening(&dch_whitened[0], dch, 160);

    return dch;
}

std::string FramePhase::treatYsfString(const std::string& input) {
    auto end = input.find_last_not_of("\n ");
    if (end == std::string::npos) return  "";
    // TODO convert to UTF-8
    return input.substr(0, end + 1);
}
