#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "ysf_trellis.h"
#include "ysf_golay.h"

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

uint8_t buf[BUF_SIZE];
uint8_t ringbuffer[RINGBUFFER_SIZE];
int ringbuffer_write_pos = 0;
int ringbuffer_read_pos = 0;

#define SYNC_SIZE 20

#define SYNCTYPE_UNKNOWN 0
#define SYNCTYPE_AVAILABLE 1

//D471C9634D
uint8_t ysf_sync[] =  { 3,1,1,0,1,3,0,1,3,0,2,1,1,2,0,3,1,0,3,1 };

#define FRAME_SIZE 480


FILE *meta_fifo = NULL;

void meta_write(char* metadata) {
    if (meta_fifo == NULL) return;
    fwrite(metadata, 1, strlen(metadata), meta_fifo);
    fflush(meta_fifo);
}

void DumpHex(const void* data, size_t size) {
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    for (i = 0; i < size; ++i) {
        fprintf(stderr, "%02X ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            fprintf(stderr, " ");
            if ((i+1) % 16 == 0) {
                fprintf(stderr, "|  %s \n", ascii);
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    fprintf(stderr, " ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    fprintf(stderr, "   ");
                }
                fprintf(stderr, "|  %s \n", ascii);
            }
        }
    }
}


// modulo that will respect the sign
unsigned int mod(int n, int x) { return ((n%x)+x)%x; }

int ringbuffer_bytes() {
    return mod(ringbuffer_write_pos - ringbuffer_read_pos, RINGBUFFER_SIZE);
}

int get_synctype(uint8_t potential_sync[SYNC_SIZE]) {
    if (memcmp(potential_sync, ysf_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_AVAILABLE;
    }
    return SYNCTYPE_UNKNOWN;
}

void main(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"fifo", required_argument, NULL, 'f'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "f:", long_options, NULL)) != -1) {
        switch (c) {
            case 'f':
                fprintf(stderr, "meta fifo: %s\n", optarg);
                meta_fifo = fopen(optarg, "w");
                break;
        }
    }

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

        while (!sync && ringbuffer_bytes() > SYNC_SIZE) {
            //fprintf(stderr, "ringbuffer_write_pos = %i; scanning ringbuffer at %i\n", ringbuffer_write_pos, i);
            if (ringbuffer_read_pos >= RINGBUFFER_SIZE) i = 0;

            uint8_t potential_sync[SYNC_SIZE];

            int k;
            for (k = 0; k < SYNC_SIZE; k++) potential_sync[k] = ringbuffer[(ringbuffer_read_pos + k) % RINGBUFFER_SIZE];

            if (get_synctype(potential_sync) != SYNCTYPE_UNKNOWN) {
                sync = true; sync_missing = 0;
                break;
            }

            ringbuffer_read_pos++;
            if (ringbuffer_read_pos >= RINGBUFFER_SIZE) ringbuffer_read_pos = 0;
        }

        while (sync && ringbuffer_bytes() > FRAME_SIZE) {
            int k;
            uint8_t potential_sync[SYNC_SIZE];
            for (k = 0; k < SYNC_SIZE; k++) potential_sync[k] = ringbuffer[(ringbuffer_read_pos + k) % RINGBUFFER_SIZE];

            int synctype = get_synctype(potential_sync);
            if (synctype != SYNCTYPE_UNKNOWN) {
                sync_missing = 0;
            } else {
                fprintf(stderr, "going to %i without sync\n", ringbuffer_read_pos);
                sync_missing++;
            }

            if (sync_missing >= 12) {
                fprintf(stderr, "lost sync at %i\n", ringbuffer_read_pos);
                sync = false;
                meta_write("\n");
                break;
            }

            uint8_t fich_raw[25] = { 0 };
            for (i = 0; i < 100; i++) {
                int offset = SYNC_SIZE + ((i * 20) % 100 + i * 20 / 100);
                int outpos = i / 4;
                int outshift = 6 - 2 * (i % 4);
                fich_raw[outpos] |= (ringbuffer[(ringbuffer_read_pos + offset) % RINGBUFFER_SIZE] & 3) << outshift;
            }

            uint8_t fich_trellis[13];
            uint8_t result = decode_trellis(&fich_raw[0], 100, &fich_trellis[0]);

            uint8_t fich_golay[4][2];
            uint32_t golay_result = 0;
            for (i = 0; i < 4; i++) {
                uint32_t g = decode_golay(&fich_trellis[i * 3], &fich_golay[i][0]);
                golay_result += g;
            }

            if (golay_result == 0) {
                // TODO there's still a CRC16 to be implemented here...

                // fich decoded without errors? accept as sync
                sync_missing = 0;

                // re-combine final fich from golay result
                uint32_t fich =
                    fich_golay[0][0] << 24 | (fich_golay[0][1] & 0xf0) << 16 |
                    fich_golay[1][0] << 12 | (fich_golay[1][1] & 0xf0) << 4 |
                    fich_golay[2][0];

                DumpHex(&fich, 4);
            } else {
                fprintf(stderr, "golay failure: %i\n", golay_result);
            }

            // advance to the next frame. as long as we have sync, we know where the next frame begins
            ringbuffer_read_pos = mod(ringbuffer_read_pos + FRAME_SIZE, RINGBUFFER_SIZE);
        }
    }
}
