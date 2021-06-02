#include <stdint.h>

// positions of TACT bits in CACH
uint8_t tact_positions[7] = { 0, 4, 8, 12, 14, 18, 22 };
// remaining CACH payload
uint8_t cach_payload_positions[17] = { 1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 15, 16, 17, 19, 20, 21, 23 };
