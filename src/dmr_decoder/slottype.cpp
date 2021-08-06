#include "slottype.hpp"

extern "C" {
#include "golay_20_8.h"
}

using namespace Digiham::Dmr;

SlotType* SlotType::parse(uint32_t raw) {
    if (!golay_20_8(&raw)) return nullptr;
    return new SlotType(raw);
}

SlotType::SlotType(uint32_t data): data(data) {}

unsigned char SlotType::getColorCode() {
    return (data >> 16) & 0b1111;
}

unsigned char SlotType::getDataType() {
    return (data >> 12) & 0b1111;
}