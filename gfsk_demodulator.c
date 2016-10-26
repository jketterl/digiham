#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

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
#define VOLUME_RB_SIZE SAMPLES_PER_SYMBOL * 10

short volume_rb[VOLUME_RB_SIZE];
int volume_rb_pos = 0;

short min = 0, max = 0, centre = 0, umid = 0, lmid = 0;

unsigned int mod(int n, int x) { return ((n%x)+x)%x; }

void calibrate_audio() {
    int i;
    min = 0, max = 0;

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
    //int zcd[SAMPLES_PER_SYMBOL] = {0};
    //int zcd_counter = 0;
    short last_value = 0;
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
                volume_rb[volume_rb_pos + i] = value;
                //fprintf(stderr, "value: %i, last_value: %i : ", value, last_value);
                /*
                if (last_value > centre ^ value > centre) {
                    zcd[i]++;
                }
                last_value = value;
                */
            }
            ringbuffer_read_pos = mod(ringbuffer_read_pos + SAMPLES_PER_SYMBOL, RINGBUFFER_SIZE);
            /*
            zcd_counter ++;
            if (zcd_counter % 100 == 0) {
                int total_crossings = 0, low = 0, high = 0;
                for (i = 0; i < SAMPLES_PER_SYMBOL; i++) {
                    total_crossings += zcd[i];
                    if (i <= SAMPLES_PER_SYMBOL / 2) {
                        high += zcd[i];
                    } else {
                        low += zcd[i];
                    }
                }
                if (total_crossings > 0) {
                    //fprintf(stderr, "zcd report: ");
                    //for (i = 0; i < SAMPLES_PER_SYMBOL; i++) fprintf(stderr, "%3i ", zcd[i]);
                    double indication = ((double) high - low) / total_crossings;
                    //if (fabs(indication) > .3) fprintf(stderr, "indication: %s (%.2f%%)", indication >= 0 ? "positive" : "negative", indication * 100);

                    //if (indication > .3d) ringbuffer_read_pos = mod(ringbuffer_read_pos + 1, RINGBUFFER_SIZE);
                    //if (indication < -.3d) ringbuffer_read_pos = mod(ringbuffer_read_pos - 1, RINGBUFFER_SIZE);
                    //fprintf(stderr, "\n");
                }
                memset(zcd, 0, sizeof(zcd));
            }
            */
            volume_rb_pos += SAMPLES_PER_SYMBOL;
            if (volume_rb_pos >= VOLUME_RB_SIZE) volume_rb_pos = 0;

            calibrate_audio();

            float average = ((float) sum) / SAMPLES_PER_SYMBOL;

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
    }
}

