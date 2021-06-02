#include <stdint.h>
#include <stdbool.h>

uint16_t crc16_checksum(uint8_t* data, int count);
bool crc16(uint8_t* data, int count, uint16_t* checksum);
