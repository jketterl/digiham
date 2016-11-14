#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#define RINGBUFFER_SIZE 1024
#define READ_SIZE 256
short ringbuffer[RINGBUFFER_SIZE];
short buf[READ_SIZE];
int ringbuffer_read_pos = 0;
int ringbuffer_write_pos = 0;
#define OUTPUT_SIZE 100
uint8_t output[OUTPUT_SIZE];
int output_pos = 0;

#define SAMPLES_PER_SYMBOL 10
#define VOLUME_RB_SIZE 100

short volume_rb[VOLUME_RB_SIZE];
int volume_rb_pos = 0;

#define VARIANCE_RB_SIZE 100 * SAMPLES_PER_SYMBOL
short variance_rb[VARIANCE_RB_SIZE];
int variance_rb_pos = 0;

short min = 0, max = 0, centre = 0, umid = 0, lmid = 0;

unsigned int mod(int n, int x) { return ((n%x)+x)%x; }

void calibrate_audio() {
    int i;
    min = SHRT_MAX, max = SHRT_MIN;

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

void main() {
    int r = 0;
    while ((r = fread(buf, 2, READ_SIZE, stdin)) > 0) {

        int i = 0;
        for (i = 0; i < r; i++) {
            ringbuffer[ringbuffer_write_pos] = buf[i];
            ringbuffer_write_pos ++;
            if (ringbuffer_write_pos >= RINGBUFFER_SIZE) ringbuffer_write_pos = 0;
        }

        output_pos = 0;
        while (ringbuffer_bytes() > SAMPLES_PER_SYMBOL + 1) {
            int sum = 0;
            for (i = 0; i < SAMPLES_PER_SYMBOL; i++) {
                short value = ringbuffer[mod(ringbuffer_read_pos + i, RINGBUFFER_SIZE)];
                sum += value;
                variance_rb[variance_rb_pos + i] = value;
            }
            ringbuffer_read_pos = mod(ringbuffer_read_pos + SAMPLES_PER_SYMBOL, RINGBUFFER_SIZE);

            variance_rb_pos += SAMPLES_PER_SYMBOL;
            if (variance_rb_pos >= VARIANCE_RB_SIZE) {

                unsigned int variance_sums[SAMPLES_PER_SYMBOL] = { 0 };
                unsigned int variance_total = 0;
                for (i = 0; i < VARIANCE_RB_SIZE; i++) {
                    variance_sums[i % SAMPLES_PER_SYMBOL] += abs(variance_rb[i]);
                    variance_total += abs(variance_rb[i]);
                }
                //fprintf(stderr, "variances: ");
                unsigned int vmin = -1;
                int vmin_pos = 0;
                for (i = 0; i < SAMPLES_PER_SYMBOL; i++) {
                    //fprintf(stderr, "%.1f ", 100.0f * variance_sums[i] / variance_total);
                    if (variance_sums[i] < vmin) {
                        vmin = variance_sums[i];
                        vmin_pos = i;
                    }
                }
                //fprintf(stderr, "\n");
                //fprintf(stderr, "minimum variance: %.1f @ %i ", 100.0f * vmin / variance_total, vmin_pos);
                if (vmin_pos > 0 && vmin_pos <= 4) {
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

            short average = sum / SAMPLES_PER_SYMBOL;
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
}

