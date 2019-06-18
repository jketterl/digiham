#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float lat;
    float lon;
} coordinate;

bool decode_gps(uint8_t* data, coordinate* result);