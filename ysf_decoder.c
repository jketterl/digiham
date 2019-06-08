#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include "ysf/trellis.h"
#include "ysf/fich.h"
#include "ysf/bitmappings.h"
#include "ysf/whitening.h"
#include "version.h"
#include "ysf/golay_24_12.h"
#include "ysf/crc16.h"

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

typedef struct {
    char mode[2];
    char dest[10];
    char src[10];
    char down[10];
    char up[10];
    char rem1[5];
    char rem2[5];
    char rem3[5];
    char rem4[5];
} call_data;

void reset_call(call_data* call) {
    memset(&call->mode[0], 0, 2);
    memset(&call->dest[0], 0, 10);
    memset(&call->src[0], 0, 10);
    memset(&call->up[0], 0, 10);
    memset(&call->down[0], 0, 10);
    memset(&call->rem1[0], 0, 5);
    memset(&call->rem2[0], 0, 5);
    memset(&call->rem3[0], 0, 5);
    memset(&call->rem4[0], 0, 5);
}

void meta_send_call(call_data* call) {
   char metadata[255];
   sprintf(metadata, "protocol:YSF;mode:%.2s;source:%.10s;target:%.10s;up:%.10s;down:%.10s\n", call->mode, call->src, call->dest, call->up, call->down);
   meta_write(&metadata[0]); 
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

uint8_t tribit_majority_table[8] = { 0, 0, 0, 1, 0, 1, 1, 1 };

void decode_tribits(uint8_t* input, uint8_t* output, uint8_t num) {
    int i, k;
    memset(output, 0, (num + 7) / 8);
    for (i = 0; i < num; i++) {
        int offset = i * 3;
        uint8_t tribit = 0;
        for (k = 0; k < 3; k++) {
            int pos = (offset + k) / 8;
            int shift = 7 - ((offset + k) % 8);

            tribit = (tribit << 1) | ((input[pos] >> shift) & 1);
        }

        int outpos = i / 8;
        int outshift = 7 - (i % 8);

        output[outpos] |= tribit_majority_table[tribit] << outshift;
    }
}


// modulo that will respect the sign
unsigned int mod(int n, int x) { return ((n%x)+x)%x; }

int ringbuffer_bytes() {
    return mod(ringbuffer_write_pos - ringbuffer_read_pos, RINGBUFFER_SIZE);
}

static const unsigned char lookuptable[256] =
{
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

int symbol_hamming_distance(uint8_t potential_sync[SYNC_SIZE]) {
    int distance = 0;
    for (int i = 0; i < SYNC_SIZE; i++) {
        uint8_t x = potential_sync[i] ^ ysf_sync[i];
        distance += lookuptable[x];
    }
    return distance;
}

int get_synctype(uint8_t potential_sync[SYNC_SIZE]) {
    if (memcmp(potential_sync, ysf_sync, SYNC_SIZE) == 0) {
        //fprintf(stderr, "found a sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_AVAILABLE;
    }
    // accept up to 3 wrong bits and still call it a sync
    int distance = symbol_hamming_distance(potential_sync);
    if (distance <= 3) {
        return SYNCTYPE_AVAILABLE;
    }
    return SYNCTYPE_UNKNOWN;
}

void deinterleave_voice_payload(uint8_t payload[9], uint8_t result[12]) {
    memset(result, 0, 12);
    int input_bit = 0;

    for (input_bit = 0; input_bit < 72; input_bit++) {
        int input_position = input_bit / 8;
        int input_shift = 7 - input_bit % 8;

        int output_bit = voice_mapping[input_bit];
        int output_position = output_bit / 8;
        int output_shift = 7 - output_bit % 8;

        int x = (payload[input_position] >> input_shift) & 1;

        // output will be blown up to 96 bits per frame
        result[output_position] |= x << output_shift;
    }
}

int main(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"fifo", required_argument, NULL, 'f'},
        {"version", no_argument, NULL, 'v'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "f:v", long_options, NULL)) != -1) {
        switch (c) {
            case 'f':
                fprintf(stderr, "meta fifo: %s\n", optarg);
                meta_fifo = fopen(optarg, "w");
                break;
            case 'v':
                print_version();
                return 0;
        }
    }

    int r = 0;
    bool sync = false;
    int sync_missing = 0;
    call_data current_call;
    reset_call(&current_call);
    fich* running_fich = 0;

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
                //fprintf(stderr, "going to %i without sync\n", ringbuffer_read_pos);
                sync_missing++;
            }

            if (sync_missing >= 12) {
                fprintf(stderr, "lost sync at %i\n", ringbuffer_read_pos);
                sync = false;
                running_fich = 0;
                reset_call(&current_call);
                meta_send_call(&current_call);
                meta_write("\n");
                break;
            }

            uint8_t fich_raw[25] = { 0 };
            // 5 by 20 deinterleave
            for (i = 0; i < 100; i++) {
                int offset = SYNC_SIZE + ((i * 20) % 100 + i * 20 / 100);
                int outpos = i / 4;
                int outshift = 6 - 2 * (i % 4);
                fich_raw[outpos] |= (ringbuffer[(ringbuffer_read_pos + offset) % RINGBUFFER_SIZE] & 3) << outshift;
            }

            uint8_t fich_trellis[13];
            uint8_t result = decode_trellis(&fich_raw[0], 100, &fich_trellis[0]);

            uint32_t fich_golay[4] = { 0 };
            bool golay_result = true;
            for (i = 0; i < 4; i++) {
                fich_golay[i] = 0 |
                                fich_trellis[i * 3] << 16 |
                                fich_trellis[i * 3 + 1] << 8 |
                                fich_trellis[i * 3 + 2];
                golay_result &= golay_24_12(&fich_golay[i]);
            }

            fich* current_fich = 0;

            if (golay_result) {
                // re-combine final fich from golay result
                uint32_t fich_data = 0 |
                    (fich_golay[0] & 0b00000000111111111111000000000000) << 8 |
                    (fich_golay[1] & 0b00000000111111111111000000000000) >> 4 |
                    (fich_golay[2] & 0b00000000111111110000000000000000) >> 16;

                uint16_t fich_checksum = 0 |
                    (fich_golay[2] & 0b00000000000000001111000000000000) |
                    (fich_golay[3] & 0b00000000111111111111000000000000) >> 12;

                if (crc16(&fich_data, &fich_checksum)) {
                    // fich decoded without errors? accept as sync
                    sync_missing = 0;

                    fich x = decode_fich(fich_data);
                    running_fich = &x;
                    current_fich = &x;

                } else {
                    fprintf(stderr, "CRC failure\n");
                }
            } else {
                fprintf(stderr, "golay failure\n");
            }

            if (running_fich > 0) {

                //fprintf(stderr, "frame type: %i, call type: %i, data type: %i, sql type: %i\n", fich.frame_type, fich.call_type, fich.data_type, fich.sql_type);

                if (running_fich->frame_type == 1) switch (running_fich->data_type) {
                    case 0:
                        // V/D mode type 1
                        // contains 5 voice channel blocks à 72 bits
                        memcpy(&current_call.mode[0], "V1", 2);
                        meta_send_call(&current_call);
                        for (i = 0; i < 5; i++) {
                            uint8_t voice[9];
                            // 20 dibits sync + 100 dibits fich + 36 dibits data channel + block offset
                            int offset = ringbuffer_read_pos + 156 + i * 72;
                            for (k = 0; k < 36; k++) {
                                uint8_t pos = k / 4;
                                uint8_t shift = 6 - 2 * (k % 4);

                                voice[pos] = (ringbuffer[(offset + k) % RINGBUFFER_SIZE] & 3) << shift;
                            }

                            uint8_t voice_frame[12];

                            deinterleave_voice_payload(voice, voice_frame);
                            fwrite(&running_fich->data_type, 1, 1, stdout);
                            fwrite(voice_frame, 1, 12, stdout);
                            fflush(stdout);
                        }
                        break;
                    case 2:
                        // V/D mode type 2
                        // contains 5 voice channel blocks à 72 (data) + 32 (check) bits
                        memcpy(&current_call.mode[0], "V2", 2);
                        meta_send_call(&current_call);
                        for (i = 0; i < 5; i++) {
                            uint8_t voice_interleaved[13] = { 0 };
                            // 20 dibits sync + 100 dibits fich + 20 dibits data channel + block offset
                            int base_offset = ringbuffer_read_pos + 140 + i * 72;
                            for (k = 0; k < 52; k++) {
                                uint8_t pos = k / 4;
                                uint8_t shift = 6 - 2 * (k % 4);

                                voice_interleaved[pos] |= (ringbuffer[(base_offset + k) % RINGBUFFER_SIZE] & 3) << shift;
                            }

                            uint8_t voice_whitened[13] = {0};
                            // 26 by 4 deinterleave
                            for (k = 0; k < 104; k++) {
                                int offset = (k * 4) % 104 + k * 4 / 104;
                                uint8_t inpos = offset / 8;
                                uint8_t inshift = 7 - offset % 8;

                                uint8_t outpos = k / 8;
                                uint8_t outshift = 7 - k % 8;

                                voice_whitened[outpos] |= ((voice_interleaved[inpos] >> inshift) & 1) << outshift;
                            }

                            uint8_t voice_tribit[13] = { 0 };
                            decode_whitening(&voice_whitened[0], &voice_tribit[0], 104);

                            uint8_t voice[7] = { 0 };
                            decode_tribits(&voice_tribit[0], &voice[0], 27);

                            // bitwise copying with offset...
                            for (k = 0; k < 22; k++) {
                                int inbit_pos = k + 81;
                                int inpos = inbit_pos / 8;
                                int inshift = 7 - (inbit_pos % 8);

                                int outbit_pos = k + 27;
                                int outpos = outbit_pos / 8;
                                int outshift = 7 - (outbit_pos % 8);

                                voice[outpos] |= ((voice_tribit[inpos] >> inshift) & 1) << outshift;
                            }

                            fwrite(&running_fich->data_type, 1, 1, stdout);
                            fwrite(voice, 1, 7, stdout);
                            fflush(stdout);
                        }
                        // contains 5 data channel blocks à 40 bits
                        uint8_t dch_raw[25] = { 0 };

                        // 20 dibits sync + 100 dibits fich
                        int base_offset = ringbuffer_read_pos + 120;
                        // 20 by 5 dibit matrix interleaving
                        for (i = 0; i < 100; i++) {
                            int pos = i / 4;
                            int shift = 6 - 2 * (i % 4);

                            int inpos = base_offset + ((i % 5) * 72 + (i * 2) / 10);

                            dch_raw[pos] |= (ringbuffer[inpos % RINGBUFFER_SIZE] & 3) << shift;
                        }

                        uint8_t dch_whitened[13] = { 0 };
                        uint8_t r = decode_trellis(&dch_raw[0], 100, &dch_whitened[0]);
                        uint8_t dch[13] = { 0 };
                        decode_whitening(&dch_whitened[0], &dch[0], 100);

                        //TODO CRC16

                        if (current_fich > 0) {
                            char* target = 0;
                            switch(current_fich->frame_number) {
                                case 0:
                                    target = &current_call.dest[0];
                                    break;
                                case 1:
                                    target = &current_call.src[0];
                                    break;
                                case 2:
                                    target = &current_call.down[0];
                                    break;
                                case 3:
                                    target = &current_call.up[0];
                                    break;
                            }

                            if (target != 0) {
                                memcpy(target, &dch[0], 10);
                                meta_send_call(&current_call);
                            //} else {
                            //    fprintf(stderr, "unprocessed data (FN=%i): ", fich.frame_number);
                            //    DumpHex(dch, 13);
                            }
                        }

                        break;
                    case 3:
                        // Voice FR mode
                        memcpy(&current_call.mode[0], "FR", 2);
                        meta_send_call(&current_call);
                        // not implemented yet
                    //case 1:
                        // Data FR mode
                        // not implemented yet
                } else if (running_fich->frame_type == 2) {
                    reset_call(&current_call);
                    meta_send_call(&current_call);
                }
            }

            // advance to the next frame. as long as we have sync, we know where the next frame begins
            ringbuffer_read_pos = mod(ringbuffer_read_pos + FRAME_SIZE, RINGBUFFER_SIZE);
        }
    }

    return 0;
}
