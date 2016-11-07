#include <stdint.h>

// there seems to be a system here, at least to some extent. but parts of it are pretty arbitrary and would result in pretty unintelligible condions, so i'll leave it as a constant here.
uint8_t voice_mapping[72] = {
    23, 5,  34, 51,
    22, 4,  33, 50,
    21, 3,  32, 49,
    20, 2,  31, 48,
    19, 1,  30, 85,
    18, 0,  29, 84,
    17, 46, 28, 83,
    16, 45, 27, 82,
    15, 44, 26, 81,
    14, 43, 25, 80,
    13, 42, 24, 79,
    12, 41, 58, 78,
    11, 40, 57, 77,
    10, 39, 56, 76,
    9,  38, 55, 75,
    8,  37, 54, 74,
    7,  36, 53, 73,
    6,  35, 52, 72
};

// positions of TACT bits in CACH
uint8_t tact_positions[7] = { 0, 4, 8, 12, 14, 18, 22 };
// remaining CACH payload
uint8_t cach_payload_positions[17] = { 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 15, 16, 17, 19, 20, 21, 23 };
