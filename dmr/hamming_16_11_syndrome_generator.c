#include "hamming_16_11.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

uint16_t correct_code = 0b0000000000000000;

void evaluate(uint16_t error_pattern) {
    uint16_t erroneous_code = correct_code ^ error_pattern;
    uint16_t parity = hamming_16_11_parity(&erroneous_code);
    fprintf(stderr, "{ %i, %i },", parity, error_pattern);
    if (!hamming_16_11(&erroneous_code)) {
        fprintf(stderr, " // incorrectable");
    } else if (erroneous_code != correct_code) {
        fprintf(stderr, " // incorrect result");
    }
    fprintf(stderr, "\n");
}

int main() {

    for (int i = 0; i < 16; i++) {
        evaluate( 1 << i );

        /*
        // seems like we can only correct single-bit errors
        for (int k = 0; k < 16; k++) {
            if (i == k) continue;
            evaluate( ( 1 << i ) ^ ( 1 << k) );
        }
        */
    }

}