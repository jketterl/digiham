#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "dmr_bitmappings.h"

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

uint8_t buf[BUF_SIZE];
uint8_t ringbuffer[RINGBUFFER_SIZE];
int ringbuffer_write_pos = 0;
int ringbuffer_read_pos = 0;

#define SYNC_SIZE 24

#define SYNCTYPE_UNKNOWN 0
#define SYNCTYPE_DATA 1
#define SYNCTYPE_VOICE 2

int synctypes[2] = { SYNCTYPE_UNKNOWN, SYNCTYPE_UNKNOWN };

uint8_t dmr_bs_data_sync[] =  { 3,1,3,3,3,3,1,1,1,3,3,1,1,3,1,1,3,1,3,3,1,1,3,1 };
uint8_t dmr_bs_voice_sync[] = { 1,3,1,1,1,1,3,3,3,1,1,3,3,1,3,3,1,3,1,1,3,3,1,3 };
uint8_t dmr_ms_data_sync[] =  { 3,1,1,1,3,1,1,3,3,3,1,3,1,3,3,3,3,1,1,3,1,1,1,3 };
uint8_t dmr_ms_voice_sync[] = { 1,3,3,3,1,3,3,1,1,1,3,1,3,1,1,1,1,3,3,1,3,3,3,1 };


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

int get_synctype(uint8_t potential_sync[24]) {
    if (memcmp(potential_sync, dmr_bs_data_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a bs data sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_DATA;
    }
    if (memcmp(potential_sync, dmr_bs_voice_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a bs voice sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_VOICE;
    }
    if (memcmp(potential_sync, dmr_ms_data_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a ms data sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_DATA;
    }
    if (memcmp(potential_sync, dmr_ms_voice_sync, SYNC_SIZE) == 0) {
        fprintf(stderr, "found a ms voice sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_VOICE;
    }
    return SYNCTYPE_UNKNOWN;
}

void deinterleave_voice_payload(uint8_t payload[27], uint8_t result[36]) {
    memset(result, 0, 36);
    int frame = 0;
    int input_bit = 0;

    // 27 bytes of payload contain 3 vocoder frames à 72 bits
    for (frame = 0; frame < 3; frame++) {
        for (input_bit = 0; input_bit < 72; input_bit++) {
            int input_position = input_bit / 8;
            int input_shift = 7 - input_bit % 8;

            int output_bit = voice_mapping[input_bit];
            int output_position = output_bit / 8;
            int output_shift = 7 - output_bit % 8;

            int x = (payload[frame * 9 + input_position] >> input_shift) & 1;

            // output will be blown up to 96 bits per frame
            result[frame * 12 + output_position] |= x << output_shift;
        }
    }
}

void main() {
    int r = 0;
    bool sync = false;
    int sync_missing = 0;
    FILE *fp = fopen("/tmp/payload.data", "w");
    fputs(".amb", fp);
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

            if (get_synctype(potential_sync) != SYNCTYPE_UNKNOWN) {
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

            int synctype = get_synctype(potential_sync);
            if (synctype != SYNCTYPE_UNKNOWN) {
                sync_missing = 0;
            } else {
                fprintf(stderr, "going to %i without sync\n", ringbuffer_read_pos);
                sync_missing ++;
            }

            if (sync_missing >= 12) {
                fprintf(stderr, "lost sync at %i\n", ringbuffer_read_pos);
                sync = false;
                synctypes[0] = SYNCTYPE_UNKNOWN; synctypes[1] = SYNCTYPE_UNKNOWN;
                break;
            }

            uint8_t tact = 0;
            // CACH starts 54 (payload area) + 12 (cach itself) dibits before the sync
            int cach_start = ringbuffer_read_pos - (54 + 12);
            // extract TACT bits from CACH
            for (k = 0; k < 7; k++) {
                int pos = k * 2;
                if (pos > 8) pos--;
                tact = (tact << 1) | ((ringbuffer[(cach_start + pos) % RINGBUFFER_SIZE] & 2) >> 1);
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

            uint8_t slot = (tact & 32) >> 5;

            if (synctype != SYNCTYPE_UNKNOWN) synctypes[slot] = synctype;

            fprintf(stderr, "  slot: %i busy: %i, lcss: %i\n", slot, (tact & 64) >> 6, (tact & 24) >> 3);

            // extract payload data
            uint8_t payload[27] = {0};
            // first half
            int payload_start = ringbuffer_read_pos - 54;
            for (k = 0; k < 54; k++) {
                payload[k / 4] |= (ringbuffer[(payload_start + k) % RINGBUFFER_SIZE] & 3) << (6 - 2 * (k % 4));
            }

            // second half
            payload_start = ringbuffer_read_pos + SYNC_SIZE;
            for (k = 0; k < 54; k++) {
                payload[(k + 54) / 4] |= (ringbuffer[(payload_start + k) % RINGBUFFER_SIZE]) << (6 - 2 * ((k + 54) % 4));
            }

            if (synctypes[slot] == SYNCTYPE_VOICE) {
                //DumpHex(payload, 27);
                uint8_t deinterleaved[36];
                deinterleave_voice_payload(payload, deinterleaved);
                //DumpHex(&deinterleaved, 36);
                fwrite(deinterleaved, 1, 36, fp);
                fflush(fp);
            }

            // advance to the next frame. as long as we have sync, we know where the next frame begins
            ringbuffer_read_pos = mod(ringbuffer_read_pos + 144, RINGBUFFER_SIZE);
        }
    }
}
