#include "lich.hpp"

using namespace Digiham::Nxdn;

Lich* Lich::parse(unsigned char* raw) {
    unsigned char lich_bits[8] = { 0 };

    // "Dividing"
    // dibit to bit conversion, dumping the lower bits
    for (int i = 0; i < 8; i++) {
        lich_bits[i] = (raw[i] >> 1) & 1;
    }

    // Simple check bit, only covers the 4 most significant bits
    unsigned char checkBit = 0;
    for (int i = 0; i < 4; i++) {
        checkBit ^= lich_bits[i];
    }

    // invalid checksum
    if (lich_bits[7] != checkBit) {
        return nullptr;
    }

    unsigned char lich_byte = 0;
    for (int i = 0; i < 7; i++) {
        lich_byte |= lich_bits[i] << (6 - i);
    }

    return new Lich(lich_byte);
}

Lich::Lich(unsigned char data) {
    this->data = data;
}

unsigned char Lich::getRFType() {
    return (data >> 5) & 0b11;
}

unsigned char Lich::getFunctionalType() {
    return (data >> 3) & 0b11;
}

unsigned char Lich::getOption() {
    return (data >> 1) & 0b11;
}

unsigned char Lich::getDirection() {
    return data & 1;
}