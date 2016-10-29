#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

uint8_t buf[BUF_SIZE];
uint8_t ringbuffer[RINGBUFFER_SIZE];
int ringbuffer_write_pos = 0;
int ringbuffer_read_pos = 0;

#define SYNC_SIZE 24

uint8_t dmr_bs_data_sync[] =  { 3,1,3,3,3,3,1,1,1,3,3,1,1,3,1,1,3,1,3,3,1,1,3,1 };
uint8_t dmr_bs_voice_sync[] = { 1,3,1,1,1,1,3,3,3,1,1,3,3,1,3,3,1,3,1,1,3,3,1,3 };
uint8_t dmr_ms_data_sync[] =  { 3,1,1,1,3,1,1,3,3,3,1,3,1,3,3,3,3,1,1,3,1,1,1,3 };
uint8_t dmr_ms_voice_sync[] = { 1,3,3,3,1,3,3,1,1,1,3,1,3,1,1,1,1,3,3,1,3,3,3,1 };

unsigned int mod(int n, int x) { return ((n%x)+x)%x; }

int ringbuffer_bytes() {
    return mod(ringbuffer_write_pos - ringbuffer_read_pos, RINGBUFFER_SIZE);
}

bool is_sync(uint8_t potential_sync[24]) {
    if (memcmp(potential_sync, dmr_bs_data_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a bs data sync at pos %i\n", ringbuffer_read_pos);
        return true;
    }
    if (memcmp(potential_sync, dmr_bs_voice_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a bs voice sync at pos %i\n", ringbuffer_read_pos);
        return true;
    }
    if (memcmp(potential_sync, dmr_ms_data_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a ms data sync at pos %i\n", ringbuffer_read_pos);
        return true;
    }
    if (memcmp(potential_sync, dmr_ms_voice_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a ms voice sync at pos %i\n", ringbuffer_read_pos);
        return true;
    }
    return false;
}

void main() {
    int r = 0;
    bool sync = false;
    int sync_missing = 0;
    while ((r = fread(buf, 1, BUF_SIZE, stdin)) > 0) {
        int i;
        for (i = 0; i < r; i++) {
            ringbuffer[ringbuffer_write_pos] = buf[i];
            ringbuffer_write_pos += 1;
            if (ringbuffer_write_pos >= RINGBUFFER_SIZE) ringbuffer_write_pos = 0;
        }

        //fprintf(stderr, "beginning ringbuffer scan at %i up to %i\n", ringbuffer_read_pos, ringbuffer_write_pos);
        while (!sync && ringbuffer_bytes() > SYNC_SIZE) {
            //fprintf(stderr, "ringbuffer_write_pos = %i; scanning ringbuffer at %i\n", ringbuffer_write_pos, i);
            if (ringbuffer_read_pos >= RINGBUFFER_SIZE) i = 0;

            uint8_t potential_sync[SYNC_SIZE];

            int k;
            for (k = 0; k < SYNC_SIZE; k++) potential_sync[k] = ringbuffer[(ringbuffer_read_pos + k) % RINGBUFFER_SIZE];

            if (is_sync(potential_sync)) {
                sync = true; sync_missing = 0;
                break;
            }

            ringbuffer_read_pos++;
            if (ringbuffer_read_pos >= RINGBUFFER_SIZE) ringbuffer_read_pos = 0;
        }

        while (sync && ringbuffer_bytes() > 144) {
            int k;
            uint8_t potential_sync[SYNC_SIZE];
            for (k = 0; k < SYNC_SIZE; k++) potential_sync[k] = ringbuffer[(ringbuffer_read_pos + k) % RINGBUFFER_SIZE];

            if (is_sync(potential_sync)) {
                sync_missing = 0;
            } else {
                fprintf(stderr, "going to %i without sync\n", ringbuffer_read_pos);
                sync_missing ++;
            }

            if (sync_missing >= 12) {
                fprintf(stderr, "lost sync at %i\n", ringbuffer_read_pos);
                sync = false;
                break;
            }

            uint8_t tact = 0;
            // CACH Interleaving
            for (k = 0; k < 7; k++) {
                int pos = k * 2;
                if (pos > 8) pos--;
                tact = (tact << 1) | ((ringbuffer[(ringbuffer_read_pos + 24 + 54 + pos) % RINGBUFFER_SIZE] & 2) >> 1);
            }

            // HAMMING checksum
            uint8_t hamming_matrix[] = { 7, 14, 11 };
            uint8_t checksum = 0;
            for (k = 0; k < 3; k++) {
                uint8_t masked = (tact >> 3) & hamming_matrix[k];
                checksum ^= masked;
            }

            uint8_t syndrome = (tact & 7) ^ checksum;
            tact ^= syndrome;

            /*
            if (syndrome > 0) {
                fprintf(stderr, "TACT bit error corrected");
            }
            */

            fprintf(stderr, "  slot: %i busy: %i, lcss: %i\n", (tact & 32) >> 5, (tact & 64) >> 6, (tact & 24) >> 3);

            ringbuffer_read_pos = mod(ringbuffer_read_pos + 144, RINGBUFFER_SIZE);
        }
    }
}
