#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include "version.h"
#include "hamming_distance.h"
#include "dumphex.h"
#include "pocsag/bch_31_21.h"

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

uint8_t buf[BUF_SIZE];
uint8_t ringbuffer[RINGBUFFER_SIZE];
int ringbuffer_write_pos = 0;
int ringbuffer_read_pos = 0;

#define SYNC_SIZE 32
uint8_t pocsag_sync[] =  { 0,1,1,1,1,1,0,0,1,1,0,1,0,0,1,0,0,0,0,1,0,1,0,1,1,1,0,1,1,0,0,0 };

#define CODEWORD_SIZE 32
#define CODEWORDS_PER_SYNC 16

uint32_t idle_codeword = 0b01111010100010011100000110010111;

#define MAX_MESSAGE_LENGTH 40

// modulo that will respect the sign
unsigned int mod(int n, int x) { return ((n%x)+x)%x; }

int ringbuffer_bytes() {
    return mod(ringbuffer_write_pos - ringbuffer_read_pos, RINGBUFFER_SIZE);
}

bool get_synctype(uint8_t potential_sync[SYNC_SIZE]) {
    if (symbol_hamming_distance(potential_sync, pocsag_sync, SYNC_SIZE) <= 3) {
        return true;
    }
    return false;
}

void print_usage() {
    fprintf(stderr,
        "pocsag_decoder version %s\n\n"
        "Usage: pocsag_decoder [options]\n\n"
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
    bool sync = false;
    int sync_missing = 0;
    int codeword_counter = 0;
    char* message = malloc(sizeof(char) * MAX_MESSAGE_LENGTH);
    memset(message, 0, MAX_MESSAGE_LENGTH);
    int message_pos = 0;

    while ((r = fread(buf, 1, BUF_SIZE, stdin)) > 0) {
        int i;
        for (i = 0; i < r; i++) {
            ringbuffer[ringbuffer_write_pos] = buf[i];
            ringbuffer_write_pos += 1;
            if (ringbuffer_write_pos >= RINGBUFFER_SIZE) ringbuffer_write_pos = 0;
        }

        while (!sync && ringbuffer_bytes() > SYNC_SIZE) {
            if (ringbuffer_read_pos >= RINGBUFFER_SIZE) i = 0;

            uint8_t potential_sync[SYNC_SIZE];

            int k;
            for (k = 0; k < SYNC_SIZE; k++) potential_sync[k] = ringbuffer[(ringbuffer_read_pos + k) % RINGBUFFER_SIZE];

            if (get_synctype(potential_sync)) {
                fprintf(stderr, "found sync at %i\n", ringbuffer_read_pos);
                sync = true; codeword_counter=0; sync_missing = 0;
                break;
            }

            ringbuffer_read_pos++;
            if (ringbuffer_read_pos >= RINGBUFFER_SIZE) ringbuffer_read_pos = 0;
        }

        while (sync && ringbuffer_bytes() > CODEWORD_SIZE) {
            if (codeword_counter > CODEWORDS_PER_SYNC) {
                uint8_t potential_sync[SYNC_SIZE];
                for (i = 0; i < SYNC_SIZE; i++) potential_sync[i] = ringbuffer[(ringbuffer_read_pos + i) % RINGBUFFER_SIZE];
                if (get_synctype(potential_sync)) {
                    sync_missing = 0;
                } else {
                    sync_missing++;
                }

                if (sync_missing >= 1) {
                    fprintf(stderr, "lost sync at %i\n", ringbuffer_read_pos);
                    sync = false;
                    break;
                }
                codeword_counter = 0;
                ringbuffer_read_pos = mod(ringbuffer_read_pos + SYNC_SIZE, RINGBUFFER_SIZE);
            } else {
                uint32_t codeword = 0;
                for (i = 0; i < CODEWORD_SIZE; i++) {
                    codeword |= (ringbuffer[(ringbuffer_read_pos + i) % RINGBUFFER_SIZE] && 0b1) << (31 - i);
                }
                if (memcmp(&codeword, &idle_codeword, CODEWORD_SIZE / 8) == 0) {
                    fprintf(stderr, "idle codeword\n");
                    if (message_pos > 0) DumpHex(message, 40);
                    message_pos = 0;
                    memset(message, 0, MAX_MESSAGE_LENGTH);
                } else {
                    uint32_t codeword_payload = codeword >> 1;
                    if (bch_31_21(&codeword_payload)) {
                        // TODO parity
                        uint32_t data = codeword_payload >> 10;
                        if (data & 0x100000) {
                            if (message_pos < MAX_MESSAGE_LENGTH * 7) {
                                for (i = 0; i < 20; i++) {
                                    bool bit = (data >> (19 - i)) & 0b1;
                                    message[message_pos / 7] |= bit << (message_pos % 7);
                                    message_pos ++;
                                }
                            }
                        } else {
                            if (message_pos > 0) DumpHex(message, 40);
                            message_pos = 0;
                            memset(message, 0, MAX_MESSAGE_LENGTH);

                            // 18 bits from the data
                            // the 3 last bits come from the frame position
                            uint32_t address = ((data & 0xFFFFC) << 1) | (codeword_counter / 2);
                            uint8_t function = data & 0b11;
                            fprintf(stderr, "address codeword; address = %i, function = %i\n", address, function);
                        }
                    } else {
                        fprintf(stderr, "ecc failure\n");
                    }
                }

                codeword_counter++;
                ringbuffer_read_pos = mod(ringbuffer_read_pos + CODEWORD_SIZE, RINGBUFFER_SIZE);
            }
        }
    }

    return 0;
}
