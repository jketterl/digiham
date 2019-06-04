#include <mbelib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "version.h"

int main(int argc, char** argv) {
    bool yaesu = false;
    bool use_float = false;
    int mode = 0;
    int unvoiced_quality = 1;

    int c;
    static struct option long_options[] = {
        {"yaesu", no_argument, NULL, 'y'},
        {"unvoiced-quality", required_argument, NULL, 'u'},
        {"version", no_argument, NULL, 'v'},
        {"float", no_argument, NULL, 'f'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "yu:vf", long_options, NULL)) != -1 ) {
        switch (c) {
            case 'y':
                fprintf(stderr, "enabling codec switching support for yaesu\n");
                yaesu = true;
                break;
            case 'u':
                unvoiced_quality = atoi(optarg);
                fprintf(stderr, "unvoiced quality set to %d\n", unvoiced_quality);
                break;
            case 'f':
                fprintf(stderr, "mbe_synthesizer: switching to floating point operation\n");
                use_float = true;
                break;
            case 'v':
                print_version();
                return 0;
        }
    }


    mbe_parms *current, *previous, *previous_enhanced;
    current = malloc(sizeof(mbe_parms));
    previous = malloc(sizeof(mbe_parms));
    previous_enhanced = malloc(sizeof(mbe_parms));
    
    mbe_initMbeParms(current, previous, previous_enhanced);

    int r, framesize;
    short output_short[160];
    float output_float[160];
    int errs = 0, errs2 = 0;
    char err_str[64];
    char ambe_d[49];

    bool cont = true;

    while (cont) {
        if (yaesu) {
            r = fread(&mode, 1, 1, stdin);
            if (r <= 0) {
                cont = false;
                break;
            }
        }

        switch (mode) {
            case 0:
                framesize = 12;
                break;
            case 2:
                framesize = 7;
                break;
            case 3:
                framesize = 18;
                break; 
        }

        uint8_t* buf = (uint8_t*) malloc(framesize);

        r = fread(buf, 1, framesize, stdin);
        if (r <= 0) {
            cont = false;
            break;
        }

        char ambe_d[49];
        int i, k;
        char ambe_fr[4][24];

        switch (mode) {
            case 0:
                for (i = 0; i < 4; i++) {
                    for (k = 0; k < 24; k++) {
                        int bit = i * 24 + k;
                        int position = bit / 8;
                        int shift = 7 - bit % 8;

                        ambe_fr[i][k] = (buf[position] >> shift) & 1;
                    }
                }
                if (use_float) {
                    mbe_processAmbe3600x2450Framef(output_float, &errs, &errs2, err_str, ambe_fr, ambe_d, current, previous, previous_enhanced, unvoiced_quality);
                } else {
                    mbe_processAmbe3600x2450Frame(output_short, &errs, &errs2, err_str, ambe_fr, ambe_d, current, previous, previous_enhanced, unvoiced_quality);
                }
                break;
            case 2:
                for (i = 0; i < 49; i++) {
                    int pos = i / 8;
                    int shift = 7 - i % 8;

                    ambe_d[i] = (buf[pos] >> shift) & 1;
                }
                if (use_float) {
                    mbe_processAmbe2450Dataf(output_float, &errs, &errs2, err_str, ambe_d, current, previous, previous_enhanced, unvoiced_quality);
                } else {
                    mbe_processAmbe2450Data(output_short, &errs, &errs2, err_str, ambe_d, current, previous, previous_enhanced, unvoiced_quality);
                }
                break;
        }

        fprintf(stderr, "%s", err_str);

        if (use_float) {
            fwrite(output_float, 4, 160, stdout);
        } else {
            fwrite(output_short, 2, 160, stdout);
        }
        fflush(stdout);
    }

    return 0;

// void mbe_processAmbe3600x2450Framef (float *aout_buf, int *errs, int *errs2, char *err_str, char ambe_fr[4][24], char ambe_d[49], mbe_parms * cur_mp, mbe_parms * prev_mp, mbe_parms * prev_mp_enhanced, int uvquality);

// mbe_processAmbe2450Dataf (float *aout_buf, int *errs, int *errs2, char *err_str, char ambe_d[49], mbe_parms * cur_mp, mbe_parms * prev_mp, mbe_parms * prev_mp_enhanced, int uvquality)


}
