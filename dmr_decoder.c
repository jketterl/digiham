#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <stdlib.h>
#include "dmr/bitmappings.h"
#include "version.h"
#include "dmr/quadratic_residue.h"
#include "dmr/hamming_16_11.h"
#include "dmr/hamming_7_4.h"
#include "dmr/golay_20_8.h"
#include "dmr/hamming_13_9.h"
#include "dmr/hamming_15_11.h"
#include "dmr/bptc_196_96.h"
#include "hamming_distance.h"

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

// data types according to ETSI 9.3.6
#define DATA_TYPE_PI 0
#define DATA_TYPE_VOICE_LC 1
#define DATA_TYPE_TERMINATOR_LC 2
#define DATA_TYPE_CSBK 3
#define DATA_TYPE_MBC 4
#define DATA_TYPE_MBC_CONTINUATION 5
#define DATA_TYPE_DATA_HEADER 6
#define DATA_TYPE_RATE_1_2_DATA 7
#define DATA_TYPE_RATE_3_4_DATA 8
#define DATA_TYPE_IDLE 9
#define DATA_TYPE_RATE_1_DATA 10
#define DATA_TYPE_UNIFIED_SINGLE_BLOCK_DATA 11
// the remainder is reserved, leaving them out for now


FILE *meta_fifo = NULL;
FILE *control_fifo = NULL;

typedef struct {
    char* sync;
    char* type;
    uint32_t source;
    uint32_t target;
} meta_struct;

meta_struct metadata[2];

void meta_write(uint8_t slot) {
    if (meta_fifo == NULL) return;
    meta_struct* meta = &metadata[slot];
    char meta_string[255];
    char builder[255];

    sprintf(meta_string, "protocol:DMR;slot:%i;", slot);
    if (meta->sync != NULL && strlen(meta->sync) > 0) {
        sprintf(builder, "sync:%s;", meta->sync);
        strcat(meta_string, builder);
    }
    if (meta->type != NULL && strlen(meta->type) > 0) {
        sprintf(builder, "type:%s;", meta->type);
        strcat(meta_string, builder);
    }
    if (meta->source > 0) {
        sprintf(builder, "source:%i;", meta->source);
        strcat(meta_string, builder);
    }
    if (meta->target > 0) {
        sprintf(builder, "target:%i;", meta->target);
        strcat(meta_string, builder);
    }
    strcat(meta_string, "\n");
    fwrite(meta_string, 1, strlen(meta_string), meta_fifo);
    fflush(meta_fifo);
}

void meta_reset(uint8_t slot) {
    meta_struct* meta = &metadata[slot];
    meta->sync = "";
    meta->type = "";
    meta->source = 0;
    meta->target = 0;
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
    if (symbol_hamming_distance(potential_sync, dmr_bs_data_sync, SYNC_SIZE) <= 3) {
        //fprintf(stderr, "found a bs data sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_DATA;
    }
    if (symbol_hamming_distance(potential_sync, dmr_bs_voice_sync, SYNC_SIZE) <= 3) {
        //fprintf(stderr, "found a bs voice sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_VOICE;
    }
    if (symbol_hamming_distance(potential_sync, dmr_ms_data_sync, SYNC_SIZE) <= 3) {
        //fprintf(stderr, "found a ms data sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_DATA;
    }
    if (symbol_hamming_distance(potential_sync, dmr_ms_voice_sync, SYNC_SIZE) <= 3) {
        //fprintf(stderr, "found a ms voice sync at pos %i\n", ringbuffer_read_pos);
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

#define LCSS_SINGLE 0
#define LCSS_START 1
#define LCSS_STOP 2
#define LCSS_CONTINUATION 3

uint8_t lcss_backlog[12];
uint8_t lcss_position = 0;

void reset_cach_payload() {
    lcss_position = 0;
    memset(lcss_backlog, 0, 12);
}

void copy_lcss_bits(uint8_t payload[3], uint8_t offset) {
    int i;
    for (i = 0; i < 17; i++) {
        int input_pos = i / 8;
        int input_shift = 7 - i % 8;

        int output_bit = offset * 17 + i;
        int output_pos = output_bit / 8;
        int output_shift = 7 - output_bit % 8;

        lcss_backlog[output_pos] |= ((payload[input_pos] >> input_shift) & 1) << output_shift;
    }
}

void decode_lcss_bits(uint8_t bits[], uint8_t count) {
    // TODO BPTC FEC
}

void collect_cach_payload(uint8_t payload[3], uint8_t lcss) {
    switch (lcss) {
        case LCSS_SINGLE:
            decode_lcss_bits(payload, 17);
            break;

        case LCSS_START:
            reset_cach_payload();
            // break intentionally omitted

        case LCSS_CONTINUATION:
            copy_lcss_bits(payload, lcss_position++);
            break;

        case LCSS_STOP:
            copy_lcss_bits(payload, lcss_position++);
            decode_lcss_bits(lcss_backlog, lcss_position * 17);
            reset_cach_payload();
    }
}

uint8_t embedded_backlog[2][16];
uint8_t embedded_position[2] = {0};

void reset_embedded_data(uint8_t slot) {
    embedded_position[slot] = 0;
    memset(embedded_backlog[slot], 0, 16);
}

void copy_embedded_data(uint8_t embedded_data[16], uint8_t slot, uint8_t position) {
    if (position > 3) {
        fprintf(stderr, "embedded data backlog overflow; ignoring frame data\n");
        return;
    }
    int i = 0;
    for (i = 0; i < 16; i++) {
        int pos = i / 4;
        int shift = 6 - (i % 4) * 2;
        embedded_backlog[slot][position * 4 + pos] |= embedded_data[i] << shift;
    }
}

#define FLC_OPCODE_GROUP 0
#define FLC_OPCODE_UNIT_TO_UNIT 3

void decode_lc(uint8_t lc[9], uint8_t slot) {
    uint8_t flc_opcode = lc[0] & 0b00111111;
    uint32_t target_id = lc[3] << 16 | lc[4] << 8 | lc[5];
    uint32_t source_id = lc[6] << 16 | lc[7] << 8 | lc[8];
    switch (flc_opcode) {
        case FLC_OPCODE_GROUP:
            fprintf(stderr, " group voice; group: %i, source: %i ", target_id, source_id);
            metadata[slot].type = "group";
            metadata[slot].source = source_id;
            metadata[slot].target = target_id;
            meta_write(slot);
            break;
        case FLC_OPCODE_UNIT_TO_UNIT:
            fprintf(stderr, " direct voice; target: %i, source: %i ", target_id, source_id);
            metadata[slot].type = "direct";
            metadata[slot].source = source_id;
            metadata[slot].target = target_id;
            meta_write(slot);
            break;
        default:
            fprintf(stderr, " unknown flc opcode: %i ", flc_opcode);
    }
}

void decode_embedded_signalling_data(uint8_t slot) {
    fprintf(stderr, "decoding embedded signalling for slot %i ", slot);
    int i, k;
    uint16_t decode_matrix[8];
    for (i = 0; i < 16; i++) {
        uint8_t byte = embedded_backlog[slot][i];
        for (k = 0; k < 8; k++) {
            decode_matrix[k] = (decode_matrix[k] << 1) | ((byte >> (7 - k)) & 1);
        }
    }

    for (i = 0; i < 7; i++) {
        if (!hamming_16_11(&decode_matrix[i])) {
            fprintf(stderr, "incorrectable error in FLC (position %i)\n", i);
            return;
        }
    }

    // column parity check as described in B.2.1
    uint16_t parity = 0;
    for (i = 0; i < 8; i++) {
        parity ^= decode_matrix[i];
    }
    if (parity != 0) {
        fprintf(stderr, "FLC parity error: %i\n", parity);
        return;
    }

    // de-interleave according to etsi B.2.1
    uint8_t lc[9];
    lc[0] =  (decode_matrix[0] & 0b1111111100000000) >> 8;
    lc[1] =  (decode_matrix[0] & 0b0000000011100000)       | ((decode_matrix[1] & 0b1111100000000000) >> 11);
    lc[2] = ((decode_matrix[1] & 0b0000011111100000) >> 3) | ((decode_matrix[2] & 0b1100000000000000) >> 14);
    lc[3] =  (decode_matrix[2] & 0b0011111111000000) >> 6;
    lc[4] =  (decode_matrix[3] & 0b1111111100000000) >> 8;
    lc[5] =  (decode_matrix[3] & 0b0000000011000000)       | ((decode_matrix[4] & 0b1111110000000000) >> 10);
    lc[6] = ((decode_matrix[4] & 0b0000001111000000) >> 2) | ((decode_matrix[5] & 0b1111000000000000) >> 12);
    lc[7] = ((decode_matrix[5] & 0b0000111111000000) >> 4) | ((decode_matrix[6] & 0b1100000000000000) >> 14);
    lc[8] =  (decode_matrix[6] & 0b0011111111000000) >> 6;

    // checksum calculation according to etsi B.2.1 and B.3.11
    uint16_t checksum = 0;
    for (i = 0; i < 9; i++) {
        checksum += lc[i];
    }
    uint8_t checksum_mod = checksum % 31;

    uint8_t received_checksum = 0;
    for (i = 0; i < 5; i++) {
        received_checksum |= (decode_matrix[i+2] & 0b0000000000100000) >> (i + 1);
    }

    if (checksum_mod != received_checksum) {
        fprintf(stderr, "FLC checksum error; checksum: %i, received checksum: %i\n", checksum_mod, received_checksum);
        return;
    }

    decode_lc(lc, slot);
}

void collect_embedded_data(uint8_t embedded_data[16], uint8_t slot, uint8_t lcss) {
    switch (lcss) {
        case LCSS_SINGLE:
            // RC data. no idea what to do with that yet.
            break;

        case LCSS_START:
            reset_embedded_data(slot);
            // break intentionally omitted

        case LCSS_CONTINUATION:
            copy_embedded_data(embedded_data, slot, embedded_position[slot]++);
            break;

        case LCSS_STOP:
            copy_embedded_data(embedded_data, slot, embedded_position[slot]);
            if (embedded_position[slot] == 3) {
                decode_embedded_signalling_data(slot);
            }
            reset_embedded_data(slot);
            break;
    }
}

void print_usage() {
    fprintf(stderr,
        "dmr_decoder version %s\n\n"
        "Usage: dmr_decoder [options]\n\n"
        "Available options:\n"
        " -h, --help          show this message\n"
        " -f, --fifo          send metadata to this file\n"
        " -c, --control-fifo  read control messages from this file\n"
        " -v, --version       print version and exit\n",
        VERSION
    );
}

int main(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"fifo", required_argument, NULL, 'f'},
        {"version", no_argument, NULL, 'v'},
        {"control-fifo", required_argument, NULL, 'c'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "hf:vc:", long_options, NULL)) != -1) {
        switch (c) {
            case 'f':
                fprintf(stderr, "meta fifo: %s\n", optarg);
                meta_fifo = fopen(optarg, "w");
                break;
            case 'c':
                fprintf(stderr, "control fifo: %s\n", optarg);
                control_fifo = fopen(optarg, "r");
                int flags = fcntl(fileno(control_fifo), F_GETFL, 0);
                fcntl(fileno(control_fifo), F_SETFL, flags | O_NONBLOCK);
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
    int lastslot = -1;
    signed int slotstability = 0;
    uint8_t slot_filter = 3;
    int active_slot = -1;

    char * control_line;
    size_t control_bufsize = 32;
    control_line = (char *) malloc(control_bufsize * sizeof(char));

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

            bool emb_valid = false;
            uint8_t emb_data = 0;
            // embedded signaling takes the place of the sync
            if (synctype != SYNCTYPE_UNKNOWN) {
                sync_missing = 0;
            } else {
                uint16_t emb = 0;

                // try to decode as embedded signalling
                for (k = 0; k < 4; k++) {
                    emb = (emb << 2) | ringbuffer[(ringbuffer_read_pos + k) % RINGBUFFER_SIZE];
                }
                for (k = 0; k < 4; k++) {
                    emb = (emb << 2) | ringbuffer[(ringbuffer_read_pos + 20 + k) % RINGBUFFER_SIZE];
                }

                emb_valid = quadratic_residue(&emb);

                if (emb_valid) {
                    // if the EMB decoded correctly, that counts towards the sync :)
                    sync_missing = 0;
                    emb_data = (emb & 0xFE00) >> 9;
                } else {
                    sync_missing ++;
                }

            }

            if (sync_missing >= 5) {
                fprintf(stderr, "lost sync at %i\n", ringbuffer_read_pos);
                sync = false;
                synctypes[0] = SYNCTYPE_UNKNOWN; synctypes[1] = SYNCTYPE_UNKNOWN;
                reset_cach_payload();
                reset_embedded_data(0); reset_embedded_data(1);
                meta_reset(0); meta_reset(1);
                meta_write(0); meta_write(1);
                lastslot = -1;
                slotstability = 0;
                break;
            }

            uint8_t tact = 0;
            // CACH starts 54 (payload area) + 12 (cach itself) dibits before the sync
            int cach_start = ringbuffer_read_pos - (54 + 12);
            // extract TACT bits from CACH
            for (k = 0; k < 7; k++) {
                uint8_t bit = tact_positions[k];
                int pos = bit / 2;
                int shift = 1 - (bit % 2);

                tact = (tact << 1) | (((ringbuffer[(cach_start + pos) % RINGBUFFER_SIZE]) >> shift) & 1);
            }

            uint8_t cach_payload[3] = {0};
            for (k = 0; k < 17; k++) {
                uint8_t bit = cach_payload_positions[k];
                int pos = bit / 2;
                int shift = 1 - (bit % 2);

                int payload_pos = k / 8;
                int payload_shift = k % 8;

                cach_payload[payload_pos] |= ((ringbuffer[(cach_start + pos) % RINGBUFFER_SIZE] >> shift) & 1) << payload_shift;
            }

            bool tact_correct = hamming_7_4(&tact);

            uint8_t slot = (tact & 32) >> 5;
            uint8_t busy = (tact & 64) >> 6;
            uint8_t lcss = (tact & 24) >> 3;

            // slots should always be alternating, but may be overridden by 100% correct tact
            uint8_t next = lastslot ^ 1;
            if (tact_correct) {
                if (slot != next) {
                    if (slotstability < 5) {
                        slotstability = 0;
                    } else if (lastslot != -1) {
                        slot = next;
                    }
                } else {
                    slotstability += 1;
                    if (slotstability > 100) slotstability = 100;
                }
            } else if (lastslot != -1) {
                if (next != slot) {
                    slotstability -= 1;
                    if (slotstability < -100) slotstability = -100;
                }
                slot = next;
            }
            lastslot = slot;

            if (synctype != SYNCTYPE_UNKNOWN && synctypes[slot] != synctype) {
                synctypes[slot] = synctype;
                // send synctype change over metadata fifo
                if (synctype == SYNCTYPE_VOICE) {
                    metadata[slot].sync = "voice";
                } else if (synctype == SYNCTYPE_DATA) {
                    meta_struct* meta = &metadata[slot];
                    meta->sync = "data";
                    meta->type = "";
                    meta->source = 0;
                    meta->target = 0;
                } else {
                    meta_reset(slot);
                }
                meta_write(slot);
            }

            fprintf(stderr, "  slot: %i busy: %i, lcss: %i", slot, busy, lcss);

            if (emb_valid) {
                uint8_t cc = (emb_data & 0b01111000) >> 3;
                uint8_t lcss = emb_data & 0b00000011;

                uint8_t embedded_data[16] = {0};
                for (k = 0; k < 16; k++) {
                    embedded_data[k] = ringbuffer[(ringbuffer_read_pos + 4 + k) % RINGBUFFER_SIZE];
                }
                fprintf(stderr, " // EMB: CC: %i, LCSS: %i ", cc, lcss); 
                collect_embedded_data(embedded_data, slot, lcss);
            }

            fprintf(stderr, "\r");
            
            collect_cach_payload(cach_payload, lcss);

            if (synctypes[slot] == SYNCTYPE_VOICE) {
                // don't output anything if the slot is muted
                // don't output anything if the other slot is active
                if (((slot + 1) & slot_filter) && (active_slot == -1 || active_slot == slot)) {
                    active_slot = slot;
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

                    uint8_t deinterleaved[36];
                    deinterleave_voice_payload(payload, deinterleaved);
                    fwrite(deinterleaved, 1, 36, stdout);
                    fflush(stdout);
                }
            } else {
                if (active_slot == slot) {
                    active_slot = -1;
                }
                if (synctypes[slot] == SYNCTYPE_DATA) {
                    uint32_t slot_type = 0;
                    for (i = 0; i < 5; i++) {
                        uint8_t dibit = ringbuffer[(ringbuffer_read_pos - 5 + i) % RINGBUFFER_SIZE];
                        slot_type = slot_type << 2 | (dibit & 0b00000011);
                    }

                    for (i = 0; i < 5; i++) {
                        uint8_t dibit = ringbuffer[(ringbuffer_read_pos + SYNC_SIZE + i) % RINGBUFFER_SIZE];
                        slot_type = slot_type << 2 | (dibit & 0b00000011);
                    }

                    if (golay_20_8(&slot_type)) {
                        uint8_t cc = (slot_type >> 16) & 0x0F;
                        uint8_t data_type = (slot_type >> 12) & 0x0F;

                        // according to ETSI 6.2, rate 3/4 data is the only kind of data that doesn't use the BPTC(196,96) FEC
                        if (data_type == DATA_TYPE_RATE_3_4_DATA) {
                            // not decoded
                            fprintf(stderr, "skipping 3/4 data frame\n");
                        } else {

                            // extract payload data
                            uint8_t payload[25] = { 0 };
                            // first half
                            int payload_start = ringbuffer_read_pos - 54;
                            for (k = 0; k < 49; k++) {
                                payload[k / 4] |= (ringbuffer[(payload_start + k) % RINGBUFFER_SIZE] & 3) << (6 - 2 * (k % 4));
                            }

                            // second half
                            payload_start = ringbuffer_read_pos + SYNC_SIZE + 5;
                            for (k = 0; k < 49; k++) {
                                payload[(k + 49) / 4] |= (ringbuffer[(payload_start + k) % RINGBUFFER_SIZE] & 3) << (6 - 2 * ((k + 49) % 4));
                            }

                            uint8_t lc[12] = { 0 };
                            if (bptc_196_96(&payload[0], &lc[0])) {
                                if (data_type == DATA_TYPE_VOICE_LC) {
                                    decode_lc(lc, slot);
                                } else if (data_type == DATA_TYPE_TERMINATOR_LC || data_type == DATA_TYPE_IDLE) {
                                    meta_struct* meta = &metadata[slot];
                                    if (strcmp("", meta->type) != 0 || meta->source > 0 || meta->target > 0) {
                                        meta->type = "";
                                        meta->source = 0;
                                        meta->target = 0;
                                        meta_write(slot);
                                    }
                                } else if (data_type == DATA_TYPE_DATA_HEADER || data_type == DATA_TYPE_RATE_1_2_DATA) {
                                    fprintf(stderr, "raw data (data_type = %i, slot = %i)\n", data_type, slot);
                                    DumpHex(&lc, 13);
                                } else {
                                    fprintf(stderr, "unhandled data_type: %i\n", data_type);
                                }
                            }

                        }
                    } else {
                        //fprintf(stderr, "slot type PDU: golay failure\n");
                    }
                }
            }

            // advance to the next frame. as long as we have sync, we know where the next frame begins
            ringbuffer_read_pos = mod(ringbuffer_read_pos + 144, RINGBUFFER_SIZE);
        }

        if (control_fifo != NULL) {
            ssize_t read;
            while ((read = fread(control_line, sizeof(char), control_bufsize, control_fifo)) >= 2) {
                if (control_line[1] == '\n') {
                    slot_filter = control_line[0] - '0';
                }
            }
        }
    }

    free(control_line);

    return 0;
}
