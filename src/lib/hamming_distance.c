#include "hamming_distance.h"

unsigned int hamming_distance(uint8_t* a, uint8_t* b, size_t size) {
    unsigned int distance = 0;
    for (int i = 0; i < size; i++) {
        uint8_t x = a[i] ^ b[i];
        distance += lookuptable[x];
    }
    return distance;
}
