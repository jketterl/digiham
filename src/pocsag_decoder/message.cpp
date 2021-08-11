#include "message.hpp"
#include <sstream>
#include <cstring>

using namespace Digiham::Pocsag;

Message::Message(uint32_t address, unsigned char type):
    address(address),
    type(type),
    content((char*) calloc(sizeof(char), MAX_MESSAGE_LENGTH))
{}

Message::~Message() {
    free(content);
}

void Message::serialize(Csdr::Writer<unsigned char> *writer) {
    if (pos == 0) return;
    std::stringstream ss;
    ss << "address:" << address << ";message:" << content << "\n";
    std::string encoded = ss.str();
    std::memcpy(writer->getWritePointer(), encoded.c_str(), encoded.length());
    writer->advance(encoded.length());
}

void Message::append(uint32_t data) {
    switch (type) {
        case 3:
            if (pos + 20 < MAX_MESSAGE_LENGTH * 7) {
                for (int i = 0; i < 20; i++) {
                    bool bit = (data >> (19 - i)) & 0b1;
                    content[pos / 7] |= bit << (pos % 7);
                    pos++;
                }
            }
            break;
        case 0:
            if (pos + 5 < MAX_MESSAGE_LENGTH) {
                for (int i = 0; i < 5; i++) {
                    char c = 0;
                    uint8_t base = (4 - i) * 4;
                    for (int k = 0; k < 4; k++) {
                        c |= ((data >> (base + k)) & 0b1) << (3 - k);
                    }
                    if (c < 0xA) {
                        c = '0' + c;
                    } else switch (c) {
                        case 0xA:
                            c = '*';
                            break;
                        case 0xB:
                            c = 'U';
                            break;
                        case 0xC:
                            c = ' ';
                            break;
                        case 0xD:
                            c = '-';
                            break;
                        case 0xE:
                            c = ')';
                            break;
                        case 0xF:
                            c = '(';
                            break;
                    }
                    content[pos++] = c;
                }
            }
            break;
    }

}