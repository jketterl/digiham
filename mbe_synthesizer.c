#include <mbelib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define FRAMESIZE 12

void main() {
    mbe_parms *current, *previous, *previous_enhanced;
    current = malloc(sizeof(mbe_parms));
    previous = malloc(sizeof(mbe_parms));
    previous_enhanced = malloc(sizeof(mbe_parms));
    
    mbe_initMbeParms(current, previous, previous_enhanced);

    int r;
    uint8_t buf[FRAMESIZE];
    short output[160];
    int errs = 0, errs2 = 0;
    char err_str[64];
    char ambe_d[49];
    while ((r = fread(buf, 1, FRAMESIZE, stdin)) > 0) {
        char ambe_d[49];

        /*
        int i;
        for (i = 0; i < 48; i++) {
            int position = i / 8;
            int shift = 7 - i % 8;

            ambe_d[i] = (buf[position] >> shift) & 1;
        }
        ambe_d[48] = 0;
        mbe_processAmbe2450Data(output, &errs, &errs2, err_str, ambe_d, current, previous, previous_enhanced, 1);
        */

        int i, k;
        char ambe_fr[4][24];
        for (i = 0; i < 4; i++) {
            for (k = 0; k < 24; k++) {
                int bit = i * 24 + k;
                int position = bit / 8;
                int shift = 7 - bit % 8;

                ambe_fr[i][k] = (buf[position] >> shift) & 1;
            }
        }
        mbe_processAmbe3600x2450Frame(output, &errs, &errs2, err_str, ambe_fr, ambe_d, current, previous, previous_enhanced, 3);

        fprintf(stderr, "mbe: %s\n", err_str);

        fwrite(output, 2, 160, stdout);
        fflush(stdout);
    }


// void mbe_processAmbe3600x2450Framef (float *aout_buf, int *errs, int *errs2, char *err_str, char ambe_fr[4][24], char ambe_d[49], mbe_parms * cur_mp, mbe_parms * prev_mp, mbe_parms * prev_mp_enhanced, int uvquality);

// mbe_processAmbe2450Dataf (float *aout_buf, int *errs, int *errs2, char *err_str, char ambe_d[49], mbe_parms * cur_mp, mbe_parms * prev_mp, mbe_parms * prev_mp_enhanced, int uvquality)


}
