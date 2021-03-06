#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
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

#define VOLUME_RB_SIZE 100

float volume_rb[VOLUME_RB_SIZE];
int volume_rb_pos = 0;

#define VARIANCE_SYMBOLS 100
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
        "fsk_demodulator version %s\n\n"
        "Usage: fsk_demodulator [options]\n\n"
        "Available options:\n"
        " -h, --help      show this message\n"
        " -v, --version   print version and exit\n"
        " -s, --samples   samples per symbol ( = audio sample rate / symbol rate; default: 40)\n"
        " -i, --invert    invert bits (used e.g in pocsag)\n",
        VERSION
    );
}

int main(int argc, char** argv) {
    int c;
    bool invert = false;
    unsigned int samples_per_symbol = 40;
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        {"invert", no_argument, NULL, 'i'},
        {"samples", required_argument, NULL, 's'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "hvis:", long_options, NULL)) != -1) {
        switch (c) {
            case 'v':
                print_version();
                return 0;
            case 'h':
                print_usage();
                return 0;
            case 'i':
                invert = true;
                break;
            case 's':
                samples_per_symbol = strtoul(optarg, NULL, 10);
                break;
        }
    }

    unsigned int variance_rb_size = VARIANCE_SYMBOLS * samples_per_symbol;
    float variance_rb[variance_rb_size];

    int lowest_eval = (int) round((float)samples_per_symbol / 3);
    int highest_eval = (int) round((float)samples_per_symbol * 2 / 3);

    int r = 0;
    while ((r = fread(buf, 4, READ_SIZE, stdin)) > 0) {

        int i = 0;
        for (i = 0; i < r; i++) {
            ringbuffer[ringbuffer_write_pos] = buf[i];
            ringbuffer_write_pos ++;
            if (ringbuffer_write_pos >= RINGBUFFER_SIZE) ringbuffer_write_pos = 0;
        }

        output_pos = 0;
        while (ringbuffer_bytes() > samples_per_symbol + 1) {
            float sum = 0;
            float volume_sum = 0;
            for (i = 0; i < samples_per_symbol; i++) {
                float value = ringbuffer[mod(ringbuffer_read_pos + i, RINGBUFFER_SIZE)];
                if (i >= lowest_eval && i < highest_eval) sum += value;
                volume_sum += value;
                variance_rb[variance_rb_pos + i] = value;
            }
            ringbuffer_read_pos = mod(ringbuffer_read_pos + samples_per_symbol, RINGBUFFER_SIZE);

            variance_rb_pos += samples_per_symbol;
            if (variance_rb_pos >= variance_rb_size) {

                double vmin;
                int vmin_pos;

                for (i = 0; i < samples_per_symbol; i++) {
                    //fprintf(stderr, "variance calc @ %i: ", i);
                    int total = 0;
                    int k;
                    for (k = 0; k < VARIANCE_SYMBOLS; k++) {
                        total += variance_rb[k * samples_per_symbol + i];
                    }
                    double mean = (float) total / VARIANCE_SYMBOLS;
                    //fprintf(stderr, "total: %i, mean: %.0f ", total, mean);

                    double dsum = 0;
                    for (k = 0; k < VARIANCE_SYMBOLS; k++) {
                        dsum += pow(mean - variance_rb[k * samples_per_symbol + i], 2);
                    }
                    double variance = dsum / VARIANCE_SYMBOLS;
                    //fprintf(stderr, "variance: %.0f\n", variance);

                    if (i == 0 || variance < vmin) {
                        vmin = variance;
                        vmin_pos = i;
                    }
                }

                //fprintf(stderr, "minimum variance: %.1f @ %i ", vmin, vmin_pos);
                if (vmin <= 0 || vmin > 5000000) {
                    //fprintf(stderr, "no variance decision\n");
                } else if (vmin_pos > 0 && vmin_pos < samples_per_symbol / 2) {
                    // variance indicates stepping to the left
                    //fprintf(stderr, "skipping\n");
                    ringbuffer_read_pos = mod(ringbuffer_read_pos + 1, RINGBUFFER_SIZE);
                } else if (vmin_pos >= samples_per_symbol / 2 && vmin_pos < samples_per_symbol - 1) {
                    // variance indicates stepping to the right
                    //fprintf(stderr, "duplicating\n");
                    ringbuffer_read_pos = mod(ringbuffer_read_pos - 1, RINGBUFFER_SIZE);
                //} else {
                    //fprintf(stderr, "variance OK\n");
                }

                variance_rb_pos %= variance_rb_size;
            }

            float volume_average = volume_sum / samples_per_symbol;
            volume_rb[volume_rb_pos] = volume_average;

            volume_rb_pos += 1;
            if (volume_rb_pos >= VOLUME_RB_SIZE) volume_rb_pos = 0;

            calibrate_audio();

            float average = sum / (highest_eval - lowest_eval);

            if (average > centre) {
                output[output_pos] = 1 ^ invert;
            } else {
                output[output_pos] = 0 ^ invert;
            }

            output_pos += 1;
        }

        fwrite(output, 1, output_pos, stdout);
        fflush(stdout);
    }

    return 0;
}

