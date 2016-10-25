#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

uint8_t buf[BUF_SIZE];
uint8_t ringbuffer[RINGBUFFER_SIZE];
int ringbuffer_pos = 0;
int ringbuffer_scanned = 0;

uint8_t dmr_bs_data_sync[] =  { 3,1,3,3,3,3,1,1,1,3,3,1,1,3,1,1,3,1,3,3,1,1,3,1 };
uint8_t dmr_bs_voice_sync[] = { 1,3,1,1,1,1,3,3,3,1,1,3,3,1,3,3,1,3,1,1,3,3,1,3 };
uint8_t dmr_ms_data_sync[] =  { 3,1,1,1,3,1,1,3,3,3,1,3,1,3,3,3,3,1,1,3,1,1,1,3 };
uint8_t dmr_ms_voice_sync[] = { 1,3,3,3,1,3,3,1,1,1,3,1,3,1,1,1,1,3,3,1,3,3,3,1 };

void main() {
    int r = 0;
    while ((r = fread(buf, 1, BUF_SIZE, stdin)) > 0) {
        int i;
        for (i = 0; i < r; i++) {
            ringbuffer[ringbuffer_pos] = buf[i];
            ringbuffer_pos += 1;
            if (ringbuffer_pos >= RINGBUFFER_SIZE) ringbuffer_pos = 0;
        }

        //fprintf(stderr, "beginning ringbuffer scan at %i up to %i\n", ringbuffer_scanned, ringbuffer_pos);
        for (i = ringbuffer_scanned; i % RINGBUFFER_SIZE != ringbuffer_pos; i++) {
            //fprintf(stderr, "scanning ringbuffer at %i\n", i);
            if (i >= RINGBUFFER_SIZE) i = 0;

            int sync_size = sizeof(dmr_bs_data_sync);
            uint8_t potential_sync[sync_size];

            int k;
            for (k = 0; k < sync_size; k++) potential_sync[k] = ringbuffer[(i + k) % RINGBUFFER_SIZE];

            if (memcmp(potential_sync, dmr_bs_data_sync, sync_size) == 0) fprintf(stderr, "found a bs data sync at pos %i\n", i);
            if (memcmp(potential_sync, dmr_bs_voice_sync, sync_size) == 0) fprintf(stderr, "found a bs voice sync at pos %i\n", i);
            if (memcmp(potential_sync, dmr_ms_data_sync, sync_size) == 0) fprintf(stderr, "found a ms data sync at pos %i\n", i);
            if (memcmp(potential_sync, dmr_ms_voice_sync, sync_size) == 0) fprintf(stderr, "found a ms voice sync at pos %i\n", i);
        }
        ringbuffer_scanned = ringbuffer_pos;
    }
}
