#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <getopt.h>
#include "version.h"

#define RINGBUFFER_SIZE 1024
#define READ_SIZE 256
float ringbuffer[RINGBUFFER_SIZE];
float buf[READ_SIZE];
int ringbuffer_read_pos = 0;
int ringbuffer_write_pos = 0;
#define OUTPUT_SIZE 100
uint8_t output[OUTPUT_SIZE];
int output_pos = 0;

#define SAMPLES_PER_SYMBOL 10
#define VOLUME_RB_SIZE 100

float volume_rb[VOLUME_RB_SIZE];
int volume_rb_pos = 0;

#define VARIANCE_SYMBOLS 100
#define VARIANCE_RB_SIZE VARIANCE_SYMBOLS * SAMPLES_PER_SYMBOL
float variance_rb[VARIANCE_RB_SIZE];
int variance_rb_pos = 0;

float min = 0, max = 0, centre = 0, umid = 0, lmid = 0;

unsigned int mod(int n, int x) { return ((n%x)+x)%x; }

void calibrate_audio() {
    int i;
    min = FLT_MAX, max = FLT_MIN;

    for (i = 0; i < VOLUME_RB_SIZE; i++) {
        if (volume_rb[i] < min) min = volume_rb[i];
        if (volume_rb[i] > max) max = volume_rb[i];
    }

    centre = (max + min) / 2;

    umid=((max-centre)*0.625)+centre;
    lmid=((min-centre)*0.625)+centre;        

    //fprintf(stderr, "min volume: %i max volume: %i centre: %i umid: %i lmid: %i\n", min, max, centre, umid, lmid);
}

int ringbuffer_bytes() {
    return mod(ringbuffer_write_pos - ringbuffer_read_pos, RINGBUFFER_SIZE);
}

void print_usage() {
    fprintf(stderr,
        "gfsk_demodulator version %s\n\n"
        "Usage: gfsk_demodulator [options]\n\n"
        "Available options:\n"
        " -h, --help      show this message\n"
        " -v, --version   print version and exit\n",
        VERSION
    );
}

int main(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
        switch (c) {
            case 'v':
                print_version();
                return 0;
            case 'h':
                print_usage();
                return 0;
        }
    }

    int r = 0;
    while ((r = fread(buf, 4, READ_SIZE, stdin)) > 0) {

        int i = 0;
        for (i = 0; i < r; i++) {
            ringbuffer[ringbuffer_write_pos] = buf[i];
            ringbuffer_write_pos ++;
            if (ringbuffer_write_pos >= RINGBUFFER_SIZE) ringbuffer_write_pos = 0;
        }

        output_pos = 0;
        while (ringbuffer_bytes() > SAMPLES_PER_SYMBOL + 1) {
            float sum = 0;
            for (i = 0; i < SAMPLES_PER_SYMBOL; i++) {
                float value = ringbuffer[mod(ringbuffer_read_pos + i, RINGBUFFER_SIZE)];
                if (i > 0 && i < 9) sum += value;
                variance_rb[variance_rb_pos + i] = value;
            }
            ringbuffer_read_pos = mod(ringbuffer_read_pos + SAMPLES_PER_SYMBOL, RINGBUFFER_SIZE);

            variance_rb_pos += SAMPLES_PER_SYMBOL;
            if (variance_rb_pos >= VARIANCE_RB_SIZE) {

                double vmin;
                int vmin_pos;

                for (i = 0; i < SAMPLES_PER_SYMBOL; i++) {
                    //fprintf(stderr, "variance calc @ %i: ", i);
                    int total = 0;
                    int k;
                    for (k = 0; k < VARIANCE_SYMBOLS; k++) {
                        total += variance_rb[k * SAMPLES_PER_SYMBOL + i];
                    }
                    double mean = (float) total / VARIANCE_SYMBOLS;
                    //fprintf(stderr, "total: %i, mean: %.0f ", total, mean);

                    double dsum = 0;
                    for (k = 0; k < VARIANCE_SYMBOLS; k++) {
                        dsum += pow(mean - variance_rb[k * SAMPLES_PER_SYMBOL + i], 2);
                    }
                    double variance = dsum / VARIANCE_SYMBOLS;
                    //fprintf(stderr, "variance: %.0f\n", variance);

                    if (i == 0 || variance < vmin) {
                        vmin = variance;
                        vmin_pos = i;
                    }
                }

                //fprintf(stderr, "minimum variance: %.1f @ %i ", vmin, vmin_pos);
                if (vmin <= 0 || vmin > 5000000) {
                    //fprintf(stderr, "no variance decision\n");
                } else if (vmin_pos > 0 && vmin_pos <= 4) {
                    // variance indicates stepping to the left
                    //fprintf(stderr, "skipping\n");
                    ringbuffer_read_pos = mod(ringbuffer_read_pos + 1, RINGBUFFER_SIZE);
                } else if (vmin_pos >= 5 && vmin_pos < 9) {
                    // variance indicates stepping to the right
                    //fprintf(stderr, "duplicating\n");
                    ringbuffer_read_pos = mod(ringbuffer_read_pos - 1, RINGBUFFER_SIZE);
                } else {
                    //fprintf(stderr, "variance OK\n");
                }

                variance_rb_pos %= VARIANCE_RB_SIZE;
            }

            float average = sum / (SAMPLES_PER_SYMBOL - 2);
            volume_rb[volume_rb_pos] = average;

            volume_rb_pos += 1;
            if (volume_rb_pos >= VOLUME_RB_SIZE) volume_rb_pos = 0;

            calibrate_audio();

            if (average > centre) {
                if (average > umid) {
                    //fprintf(stderr, "1");
                    output[output_pos] = 1;
                } else {
                    //fprintf(stderr, "0");
                    output[output_pos] = 0;
                }
            } else {
                if (average < lmid) {
                    //fprintf(stderr, "3");
                    output[output_pos] = 3;
                } else {
                    //fprintf(stderr, "2");
                    output[output_pos] = 2;
                }
            }

            output_pos += 1;
        }
        //fprintf(stderr, "\n");

        fwrite(output, 1, output_pos, stdout);
        fflush(stdout);
    }

    return 0;
}

