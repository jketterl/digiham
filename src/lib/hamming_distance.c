#include "hamming_distance.h"

int symbol_hamming_distance(uint8_t potential_sync[], uint8_t sync[], int size) {
    int distance = 0;
    for (int i = 0; i < size; i++) {
        uint8_t x = potential_sync[i] ^ sync[i];
        distance += lookuptable[x];
    }
    return distance;
}
