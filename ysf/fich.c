#include "fich.h"
#include <stdint.h>

fich decode_fich(uint32_t input) {
    fich result;

    result.frame_type          = input >> 30 & 0b11;
    result.callsign_type       = input >> 28 & 0b11;
    result.call_type           = input >> 26 & 0b11;
    result.block_number        = input >> 24 & 0b11;
    result.block_total         = input >> 22 & 0b11;
    result.frame_number        = input >> 19 & 0b111;
    result.frame_total         = input >> 16 & 0b111;
    // 1 bit reserved here
    result.frequency_deviation = input >> 14 & 0b1;
    result.message_path        = input >> 11 & 0b111;
    result.voip_path           = input >> 10 & 0b1;
    result.data_type           = input >> 8  & 0b11;
    result.sql_type            = input >> 7  & 0b1;
    result.sql_code            = input       & 0b1111111;

    return result;
}
