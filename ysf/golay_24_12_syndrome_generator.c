#include "golay_24_12.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

uint32_t correct_code = 0b000000000000000000000000;

void evaluate(uint32_t error_pattern) {
    uint32_t erroneous_code = correct_code ^ error_pattern;
    uint32_t parity = golay_24_12_parity(&erroneous_code);
    fprintf(stderr, "{ %i, %i },", parity, error_pattern);
    if (!golay_24_12(&erroneous_code)) {
        fprintf(stderr, " // incorrectable");
    } else if (erroneous_code != correct_code) {
        fprintf(stderr, " // incorrect result");
    }
    fprintf(stderr, "\n");
}

int main() {
    // up to 3 bit errors can be corrected reliably
    for (int i = 0; i < 24; i++) {
        evaluate( 1 << i );

        for (int k = 0; k < i; k++) {
            evaluate( ( 1 << i ) ^ ( 1 << k ) );

            for (int l = 0; l < k; l++) {
                evaluate( ( 1 << i ) ^ ( 1 << k ) ^ ( 1 << l ) );
            }
        }
    }

}