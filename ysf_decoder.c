#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <stdlib.h>
#include "ysf/trellis.h"
#include "ysf/fich.h"
#include "ysf/bitmappings.h"
#include "ysf/whitening.h"
#include "version.h"
#include "ysf/golay_24_12.h"
#include "ysf/crc16.h"
#include "ysf/commands.h"
#include "ysf/radio_types.h"
#include "ysf/gps.h"
#include "hamming_distance.h"

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 2048

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
    coordinate* location;
    char* radio;
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
    if (call->location != NULL) free(call->location);
    call->location = NULL;
    call->radio = NULL;
}

void meta_send_call(call_data* call) {
    char meta_string[255];
    char builder[255];
    sprintf(meta_string, "protocol:YSF;");
    if (strcmp("", call->mode) != 0) {
        sprintf(builder, "mode:%.2s;", call->mode);
        strcat(meta_string, builder);
    }
    if (strcmp("", call->src) != 0) {
        sprintf(builder, "source:%.10s;", call->src);
        strcat(meta_string, builder);
    }
    if (strcmp("", call->dest) != 0) {
        sprintf(builder, "target:%.10s;", call->dest);
        strcat(meta_string, builder);
    }
    if (strcmp("", call->up) != 0) {
        sprintf(builder, "up:%.10s;", call->up);
        strcat(meta_string, builder);
    }
    if (strcmp("", call->down) != 0) {
        sprintf(builder, "down:%.10s;", call->down);
        strcat(meta_string, builder);
    }
    if (call->location != NULL) {
        sprintf(builder, "lat:%.6f;lon:%.6f;", call->location->lat, call->location->lon);
        strcat(meta_string, builder);
    }
    if (call->radio != NULL) {
        sprintf(builder, "radio:%s;", call->radio);
        strcat(meta_string, builder);
    }
    strcat(meta_string, "\n");
    fwrite(meta_string, 1, strlen(meta_string), meta_fifo);
    fflush(meta_fifo);
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

int get_synctype(uint8_t potential_sync[SYNC_SIZE]) {
    if (symbol_hamming_distance(potential_sync, ysf_sync, SYNC_SIZE) <= 3) {
        return SYNCTYPE_AVAILABLE;
    }
    return SYNCTYPE_UNKNOWN;
}

void deinterleave_v1_voice_payload(uint8_t payload[9], uint8_t result[12]) {
    for (int i = 0; i < 12; i++) result[i] = 0;
    int input_bit = 0;

    for (input_bit = 0; input_bit < 72; input_bit++) {
        int input_position = input_bit / 8;
        int input_shift = 7 - (input_bit % 8);

        int output_bit = v1_voice_mapping[input_bit];
        int output_position = output_bit / 8;
        int output_shift = 7 - (output_bit % 8);

        uint8_t x = (payload[input_position] >> input_shift) & 1;

        // output will be blown up to 96 bits per frame
        result[output_position] |= x << output_shift;
    }
}

void deinterleave_fr_voice_payload(uint8_t* payload, uint8_t* result) {
    for (int i = 0; i < 18; i++) result[i] = 0;
    int output_bit = 0;

    for (output_bit = 0; output_bit < 144; output_bit++) {
        int input_bit = fr_voice_mapping[output_bit];
        int input_position = input_bit / 8;
        int input_shift = 7 - input_bit % 8;

        //int output_bit = fr_voice_mapping[input_bit];
        int output_position = output_bit / 8;
        int output_shift = 7 - output_bit % 8;

        uint8_t x = (payload[input_position] >> input_shift) & 1;

        result[output_position] |= x << output_shift;
    }
}

void descramble_fr_voice(uint8_t* in, uint8_t offset, uint16_t n, uint32_t seed) {
    uint32_t v = seed;
    uint8_t mask = 0;
    for (uint16_t i = offset; i < n; i++) {
        v = ((v * 173) + 13849) & 0xffff;
        mask = (mask << 1) | (v >> 15);
        if (i % 8 == 7) {
            //fprintf(stderr, "applying mask %i to offset %i\n", mask, i / 8);
            in[i / 8] ^= mask;
            mask = 0;
        }
    }
}

void print_usage() {
    fprintf(stderr,
        "ysf_decoder version %s\n\n"
        "Usage: ysf_decoder [options]\n\n"
        "Available options:\n"
        " -h, --help      show this message\n"
        " -f, --fifo      send metadata to this file\n"
        " -v, --version   print version and exit\n",
        VERSION
    );
}

int main(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"fifo", required_argument, NULL, 'f'},
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "f:vh", long_options, NULL)) != -1) {
        switch (c) {
            case 'f':
                fprintf(stderr, "meta fifo: %s\n", optarg);
                meta_fifo = fopen(optarg, "w");
                break;
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
    call_data current_call;
    current_call.location = NULL;
    reset_call(&current_call);
    fich* running_fich = NULL;
    uint8_t last_frame = 5;
    uint8_t dch_buffer[100];
    bool expect_sub_frame = false;

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
                running_fich = NULL;
                reset_call(&current_call);
                meta_send_call(&current_call);
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

                // the endianess is reversed at this point for some reason.
                // not sure if it's a good idea to just flip it around like this, but it seems to work.
                uint32_t be =  __builtin_bswap32(fich_data);

                if (crc16((uint8_t*) &be, 4, &fich_checksum)) {
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

            if (running_fich != NULL) {

                //fprintf(stderr, "frame type: %i, call type: %i, data type: %i, sql type: %i\n", fich.frame_type, fich.call_type, fich.data_type, fich.sql_type);

                // frame type 1 = communications channel (where the data / voice is)
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

                            deinterleave_v1_voice_payload(voice, voice_frame);
                            fwrite(&running_fich->data_type, 1, 1, stdout);
                            fwrite(voice_frame, 1, 12, stdout);
                            fflush(stdout);
                        }
                        break;
                    case 2:
                        // V/D mode type 2
                        // contains 5 voice channel blocks à 72 (data) + 32 (check) bits
                        memcpy(&current_call.mode[0], "DN", 2);
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

                        uint16_t checksum = dch_whitened[10] << 8 | dch_whitened[11];
                        if (crc16((uint8_t*) &dch_whitened, 10, &checksum)) {
                            uint8_t dch[13] = { 0 };
                            decode_whitening(&dch_whitened[0], &dch[0], 100);


                            if (current_fich > 0) {
                                char* target = 0;
                                uint8_t fn = current_fich -> frame_number;
                                bool meta_to_send = false;
                                switch(fn) {
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
                                    // TODO: 4 contains REM1 & REM2
                                    // TODO: 5 contains REM3 & REM4
                                }

                                if (target != 0) {
                                    memcpy(target, &dch[0], 10);
                                    meta_to_send = true;
                                }

                                // frame number should not go above 7, but you never know
                                if (fn > 5 && fn <= 7) {
                                    // ensure sequence
                                    if (last_frame + 1 == fn) {
                                        last_frame = current_fich -> frame_number;
                                        memcpy(&dch_buffer[(fn - 6) * 10], &dch[0], 10);
                                    } else {
                                        last_frame = 5;
                                    }
                                // we need 20 bytes for DT1 and DT2
                                } else if (last_frame >= 7) {
                                    //uint8_t frames = last_frame - 5;
                                    //fprintf(stderr, "restored dch data (%i frames):\n", frames);
                                    last_frame = 5;

                                    if (dch_buffer[18] == 0x03) {
                                        uint8_t checksum = 0;
                                        for (i = 0; i < 19; i++) checksum += dch_buffer[i];
                                        if (checksum == dch_buffer[19]) {
                                            uint32_t command = dch_buffer[1] << 16 | dch_buffer[2] << 8 | dch_buffer[3];
                                            if (command == COMMAND_NULL0_GPS || command == COMMAND_NULL1_GPS) {
                                                if (current_call.location != NULL) free(current_call.location);
                                                current_call.location = NULL;
                                                meta_to_send = true;
                                            } else if (command == COMMAND_SHORT_GPS) {
                                                if (current_call.location != NULL) {
                                                    free(current_call.location);
                                                    current_call.location = NULL;
                                                    meta_to_send = true;
                                                }
                                                coordinate* location = (coordinate*) malloc(sizeof(coordinate));
                                                if (decode_gps(&dch_buffer[5], location)) {
                                                    current_call.location = location;
                                                    meta_to_send = true;
                                                } else {
                                                    free(location);
                                                    fprintf(stderr, "gps decoding failed :(\n");
                                                }
                                            }

                                            uint8_t radio_id = dch_buffer[4];
                                            current_call.radio = get_radio_type(radio_id);

                                        } else {
                                            fprintf(stderr, "checksum error\n");
                                        }
                                    } else {
                                        fprintf(stderr, "invalid terminator\n");
                                    }

                                }

                                if (meta_to_send) meta_send_call(&current_call);

                            }
                        } else {
                            fprintf(stderr, "DCH CRC failure\n");
                        }


                        break;
                    case 3:
                        // Voice FR mode
                        memcpy(&current_call.mode[0], "VW", 2);
                        meta_send_call(&current_call);

                        // TODO sub header
                        int start_frame = 0;
                        if (expect_sub_frame) start_frame = 3;
                        expect_sub_frame = false;

                        // contains 5 voice channel blocks à 144 bits
                        for (i = start_frame; i < 5; i++) {
                            uint8_t voice[18] = { 0 };
                            // 20 dibits sync + 100 dibits fich + block offset
                            int offset = ringbuffer_read_pos + 120 + i * 72;
                            for (k = 0; k < 72; k++) {
                                uint8_t pos = k / 4;
                                uint8_t shift = 6 - 2 * (k % 4);

                                voice[pos] |= (ringbuffer[(offset + k) % RINGBUFFER_SIZE] & 3) << shift;
                            }

                            uint8_t voice_frame[18] = { 0 };
                            deinterleave_fr_voice_payload(voice, voice_frame);

                            uint16_t seed = (voice_frame[0] << 8) | (voice_frame[1] & 0xF0);
                            descramble_fr_voice(&voice_frame[0], 23, 144 - 7, seed);

                            fwrite(&running_fich->data_type, 1, 1, stdout);
                            fwrite(voice_frame, 1, 18, stdout);
                            fflush(stdout);
                        }

                        break;
                    //case 1:
                        // Data FR mode
                        // not implemented yet
                // frame type 0 = header channel, i.e. beginning of transmission
                // the same information is in a frame_type 2... but there's no point decoding it since we are resetting
                // it as soon as we see one
                } else if (running_fich->frame_type == 0) {
                    reset_call(&current_call);
                    for (int dch_num = 0; dch_num < 2; dch_num++) {
                        // contains 5 data channel blocks à 40 bits
                        uint8_t dch_raw[45] = { 0 };

                        // 20 dibits sync + 100 dibits fich + DCH offset
                        int base_offset = ringbuffer_read_pos + 120 + dch_num * 36;
                        // 20 by 9 dibit matrix interleaving
                        // also pulls out the bits from their 72bit blocks
                        for (i = 0; i < 180; i++) {
                            int pos = i / 4;
                            int shift = 6 - 2 * (i % 4);

                            int streampos = (i % 9) * 20 + i / 9;
                            int blockpos = (streampos / 36) * 72;
                            int blockoffset = streampos % 36;
                            int inpos = base_offset + blockpos + blockoffset;

                            dch_raw[pos] |= (ringbuffer[inpos % RINGBUFFER_SIZE] & 3) << shift;
                        }

                        uint8_t dch_whitened[23] = { 0 };
                        uint8_t r = decode_trellis(&dch_raw[0], 180, &dch_whitened[0]);

                        uint16_t checksum = (dch_whitened[20] << 8) | dch_whitened[21];
                        bool crc_valid = crc16((uint8_t *) &dch_whitened, 20, &checksum);

                        if (crc_valid) {
                            uint8_t dch[20] = { 0 };
                            decode_whitening(&dch_whitened[0], &dch[0], 160);

                            if (dch_num == 0) {
                                // CSD1 - contains Dest and Src fields
                                memcpy(&current_call.dest[0], &dch[0], 10);
                                memcpy(&current_call.src[0], &dch[10], 10);
                            } else if (dch_num == 1) {
                                // CSD2 - contains downlink and uplink fields
                                memcpy(&current_call.down[0], &dch[0], 10);
                                memcpy(&current_call.up[0], &dch[10], 10);
                            }
                            meta_send_call(&current_call);
                        } else {
                            fprintf(stderr, "header frame DCH%i CRC failure\n", dch_num + 1);
                        }
                    }
                    // TODO only set this on FR mode
                    expect_sub_frame = true;

                // frame type 2 = terminator channel, i.e. end of transmission
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
