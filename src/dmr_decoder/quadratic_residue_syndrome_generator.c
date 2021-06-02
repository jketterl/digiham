#include "quadratic_residue.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

//uint16_t correct_code = 0b1010101001000010;
//uint16_t correct_code = 0b0101010000011001;
uint16_t correct_code = 0b0000000000000000;
//uint16_t correct_code = 0b1111111001011011;

void evaluate(uint16_t error_pattern) {
    uint16_t erroneous_code = correct_code ^ error_pattern;
    uint16_t parity = quadratic_residue_parity(&erroneous_code);
    fprintf(stderr, "{ %i, %i },", parity, error_pattern);
    if (!quadratic_residue(&erroneous_code)) {
        fprintf(stderr, " // incorrectable");
    } else if (erroneous_code != correct_code) {
        fprintf(stderr, " // incorrect result");
    }
    fprintf(stderr, "\n");
}

int main() {

    for (int i = 0; i < 16; i++) {
        evaluate( 1 << i );

        for (int k = 0; k < 16; k++) {
            if (i == k) continue;
            evaluate( ( 1 << i ) ^ ( 1 << k) );
        }
    }

}