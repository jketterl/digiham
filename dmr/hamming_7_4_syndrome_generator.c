#include "hamming_7_4.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

uint8_t correct_code = 0b00000000;

void evaluate(uint8_t error_pattern) {
    uint8_t erroneous_code = correct_code ^ error_pattern;
    uint8_t parity = hamming_7_4_parity(&erroneous_code);
    fprintf(stderr, "{ %i, %i },", parity, error_pattern);
    if (!hamming_7_4(&erroneous_code)) {
        fprintf(stderr, " // incorrectable");
    } else if (erroneous_code != correct_code) {
        fprintf(stderr, " // incorrect result");
    }
    fprintf(stderr, "\n");
}

int main() {

    for (int i = 0; i < 7; i++) {
        evaluate( 1 << i );
    }

}