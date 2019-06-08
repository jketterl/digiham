#include <stdint.h>
#include <stdbool.h>

uint16_t crc16_checksum(uint32_t* data);
bool crc16(uint32_t* data, uint16_t* checksum);
