#include "pocsag_phase.hpp"
#include "codeword.hpp"

extern "C" {
#include "hamming_distance.h"
}

using namespace Digiham::Pocsag;

bool Phase::hasSync(unsigned char* data) {
    return hamming_distance((uint8_t*) data, (uint8_t*) pocsag_sync, SYNC_SIZE) <= 3;
}

int SyncPhase::getRequiredData() {
    return SYNC_SIZE;
}

Digiham::Phase* SyncPhase::process(Csdr::Reader<unsigned char> *data, Csdr::Writer<unsigned char> *output) {
    if (hasSync(data->getReadPointer())) {
        data->advance(SYNC_SIZE);
        return new CodewordPhase();
    }

    // as long as we don't find any sync, move ahead, bit by bit
    data->advance(1);
    // tell decoder that we'll continue
    return this;
}

CodewordPhase::~CodewordPhase() {
    delete currentMessage;
}

int CodewordPhase::getRequiredData() {
    return CODEWORD_SIZE;
}

Digiham::Phase *CodewordPhase::process(Csdr::Reader<unsigned char> *data, Csdr::Writer<unsigned char> *output) {
    if (codewordCounter >= CODEWORDS_PER_SYNC) {
        if (hasSync(data->getReadPointer())) {
            if (syncCount++ > 2) syncCount = 2;
        } else {
            if (syncCount-- < 0) {
                if (currentMessage != nullptr) {
                    currentMessage->serialize(serializer, output);
                }
                return new SyncPhase();
            }
        }

        data->advance(SYNC_SIZE);
        codewordCounter = 0;
    } else {
        auto codeword = Codeword::parse(data->getReadPointer());
        if (codeword != nullptr) {
            if (codeword->isIdle()) {
                if (currentMessage != nullptr) {
                    currentMessage->serialize(serializer, output);
                }
                delete currentMessage;
                currentMessage = nullptr;
            } else if (codeword->isAddressCodeword()) {
                if (currentMessage != nullptr) {
                    currentMessage->serialize(serializer, output);
                }
                delete currentMessage;
                currentMessage = nullptr;

                unsigned char type = codeword->getFunctionBits();
                if (type == 1 || type == 3) {
                    // 18 bits from the data
                    // the 3 last bits come from the frame position
                    uint32_t address = (codeword->getAddress() << 3) | (codewordCounter / 2);
                    currentMessage = new Message(address, type);
                }
            } else {
                if (currentMessage != nullptr) {
                    currentMessage->append(codeword->getPayload());
                }
            }
            delete codeword;
        } else {
            delete currentMessage;
            currentMessage = nullptr;
        }

        data->advance(CODEWORD_SIZE);
        codewordCounter++;
    }

    return this;
}

void CodewordPhase::setSerializer(Digiham::Serializer *serializer) {
    this->serializer = serializer;
}

void CodewordPhase::writeMessage(Message *message) {

}