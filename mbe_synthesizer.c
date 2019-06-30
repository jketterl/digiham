#include <mbelib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include "version.h"

void print_usage() {
    fprintf(stderr,
        "mbe_synthesizer version %s\n\n"
        "Usage: mbe_synthesizer [options]\n\n"
        "Available options:\n"
        " -h, --help              show this message\n"
        " -v, --version           print version and exit\n"
        " -u, --unvoiced-quality  set mbelib unvoide quality level (higher values require more cpu; default: 1)\n"
        " -y, --yaesu             activate YSF mode (allows in-stream switching of different mbe codecs)\n"
        " -f, --float             output 32-bit floating-point samples (default: 16-bit signed integer)\n",
        VERSION
    );
}

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
        {"help", no_argument, NULL, 'h'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "yu:vfh", long_options, NULL)) != -1 ) {
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
            case 'h':
                print_usage();
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
    char err_str[64];

    bool cont = true;

    while (cont) {
        int errs = 0, errs2 = 0;
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

        int i, k;
        char ambe_fr[4][24] = { 0 };
        char ambe_d[49] = { 0 };
        char imbe_fr[8][23] = { 0 };
        char imbe_d[88] = { 0 };

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

            case 3:
                // let's ignore error correction for a moment here...

                for (i = 0; i < 4; i++) {
                    for (k = 0; k < 12; k++) {
                        int inpos = (i * 23 + k);
                        int pos = inpos / 8;
                        int shift = 7 - inpos % 8;

                        int outpos = i * 12 + k;

                        imbe_d[outpos] = (buf[pos] >> shift) & 1;
                    }
                }

                for (i = 0; i < 3; i++) {
                    for (k = 0; k < 11; k++) {
                        int inpos = 92 + (i * 23 + k);
                        int pos = inpos / 8;
                        int shift = 7 - inpos % 8;

                        int outpos = 48 + i * 12 + k;

                        imbe_d[outpos] = (buf[pos] >> shift) & 1;
                    }
                }

                for (k = 0; k < 7; k++) {
                    int inpos = 137 + i;
                    int pos = inpos / 8;
                    int shift = 7 - inpos % 8;

                    int outpos = 81 + k;

                    imbe_d[outpos] = (buf[pos] >> shift) & 1;
                }

                if (use_float) {
                    mbe_processImbe4400Dataf(output_float, &errs, &errs2, err_str, imbe_d, current, previous, previous_enhanced, unvoiced_quality);
                } else {
                    mbe_processImbe4400Data(output_short, &errs, &errs2, err_str, imbe_d, current, previous, previous_enhanced, unvoiced_quality);
                }

                /*
                // this should in theory pass a complete imbe frame, including ECC data. in my experience, however,
                // mbelib will discard most of the data this way, since there is too many errors in there, even when
                // the originating signal was a very strong one (i.e. taken directly from my radio). i am suspecting
                // error(s) in the deinterleaving, and maybe in the descrambling process in ysf_decoder.
                // it is very likely that the interleaving is already corrupting the data, since the first chunk of 23
                // bits is not scrambled, but is still returning with plenty of golay errors (in the errs variable).

                // first 4 are filled up completely
                for (i = 0; i < 92; i++) {
                    int chunk = i / 23;
                    int chunkpos = i % 23;
                    int pos = i / 8;
                    int shift = 7 - i % 8;

                    imbe_fr[chunk][chunkpos] = (buf[pos] >> shift) & 1;
                }
                // the rest seem to be partial
                for (i = 0; i < 52; i++) {
                    int chunk = 4 + (i / 15);
                    int chunkpos = i % 15;
                    int pos = (92 + i) / 8;
                    int shift = 7 - (92 + i) % 8;

                    imbe_fr[chunk][chunkpos] = (buf[pos] >> shift) & 1;
                }

                if (use_float) {
                    mbe_processImbe7200x4400Framef(output_float, &errs, &errs2, err_str, imbe_fr, imbe_d, current, previous, previous_enhanced, unvoiced_quality);
                } else {
                    mbe_processImbe7200x4400Frame(output_short, &errs, &errs2, err_str, imbe_fr, imbe_d, current, previous, previous_enhanced, unvoiced_quality);
                }
                */
                break;
        }

        free(buf);

        fprintf(stderr, "%s", err_str);

        if (use_float) {
            fwrite(output_float, 4, 160, stdout);
        } else {
            fwrite(output_short, 2, 160, stdout);
        }
        fflush(stdout);
    }

    return 0;
}
