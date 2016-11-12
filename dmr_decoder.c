#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>
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

int get_synctype(uint8_t potential_sync[24]) {
    if (memcmp(potential_sync, dmr_bs_data_sync, SYNC_SIZE) == 0) {
        //fprintf(stderr, "found a bs data sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_DATA;
    }
    if (memcmp(potential_sync, dmr_bs_voice_sync, SYNC_SIZE) == 0) {
        //fprintf(stderr, "found a bs voice sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_VOICE;
    }
    if (memcmp(potential_sync, dmr_ms_data_sync, SYNC_SIZE) == 0) {
        //fprintf(stderr, "found a ms data sync at pos %i\n", ringbuffer_read_pos);
        return SYNCTYPE_DATA;
    }
    if (memcmp(potential_sync, dmr_ms_voice_sync, SYNC_SIZE) == 0) {
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

void decode_embedded_data(slot) {
    fprintf(stderr, "decoding embedded data for slot %i", slot);
    int i, k;
    uint16_t decode_matrix[8];
    for (i = 0; i < 16; i++) {
        uint8_t byte = embedded_backlog[slot][i];
        for (k = 0; k < 8; k++) {
            decode_matrix[k] = (decode_matrix[k] << 1) | ((byte >> (7 - k)) & 1);
        }
    }

    // TODO flc BPTC

    uint8_t flc_opcode = (decode_matrix[0] & 0x3F00) >> 8;
    uint32_t target_id = 0;
    uint32_t source_id = 0;
    // bits are spread across the matrix rather strangely; see B.2.1
    target_id =  (decode_matrix[2] & 0b0011111111000000) << 10;
    target_id |= (decode_matrix[3] & 0b1111111111000000);
    target_id |= (decode_matrix[4] & 0b1111110000000000) >> 10;
    source_id =  (decode_matrix[4] & 0b0000001111000000) << 14;
    source_id |= (decode_matrix[5] & 0b1111111111000000) << 4;
    source_id |= (decode_matrix[6] & 0b1111111111000000) >> 6;
    char metadata[255];
    switch (flc_opcode) {
        case FLC_OPCODE_GROUP:
            fprintf(stderr, " group voice; group: %i, source: %i ", target_id, source_id);
            sprintf(metadata, "protocol:DMR;slot:%i;type:group;source:%i;target:%i\n", slot, source_id, target_id);
            meta_write(&metadata[0]);
            break;
        case FLC_OPCODE_UNIT_TO_UNIT:
            fprintf(stderr, " direct voice; target: %i, source: %i ", target_id, source_id);
            sprintf(metadata, "protocol:DMR;slot:%i;type:direct;source:%i;target:%i\n", slot, source_id, target_id);
            meta_write(&metadata[0]);
            break;
        default:
            fprintf(stderr, " unknown flc opcode: %i ", flc_opcode);
    }
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
                decode_embedded_data(slot);
            }
            reset_embedded_data(slot);
            break;
    }
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
            bool emb_present = false;
            uint8_t emb_data = 0;
            if (synctype != SYNCTYPE_UNKNOWN) {
                sync_missing = 0;
            } else {
                //fprintf(stderr, "going to %i without sync\n", ringbuffer_read_pos);
                short emb = 0;

                // try to decode as embedded signalling
                for (k = 0; k < 4; k++) {
                    emb = (emb << 2) | ringbuffer[(ringbuffer_read_pos + k) % RINGBUFFER_SIZE];
                }
                for (k = 0; k < 4; k++) {
                    emb = (emb << 2) | ringbuffer[(ringbuffer_read_pos + 20 + k) % RINGBUFFER_SIZE];
                }

                // check emb header
                short parity = 0;
                uint8_t data = (emb & 0xFE00) >> 9;
                for (k = 0; k < 9; k++) {
                    int bit = 0, l;
                    for (l = 0; l < 7; l++) {
                        if ((emb_qr_matrix[k] >> l) & 1) {
                            bit ^= ((data >> l) & 1);
                        }
                    }
              
                    parity = (parity << 1) | (bit & 1);
                }

                short parity_input = emb & 0x01FF;
                if (parity == parity_input) {
                    // if the EMB decoded correctly, that counts towards the sync :)
                    sync_missing = 0;
                    emb_present = true;
                    emb_data = data;
                } else {
                    sync_missing ++;
                    emb_present = false;
                }


            }

            if (sync_missing >= 12) {
                fprintf(stderr, "lost sync at %i\n", ringbuffer_read_pos);
                sync = false;
                synctypes[0] = SYNCTYPE_UNKNOWN; synctypes[1] = SYNCTYPE_UNKNOWN;
                reset_cach_payload();
                reset_embedded_data(0); reset_embedded_data(0);
                meta_write("\n");
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

                tact = (tact << 1) | ((ringbuffer[(cach_start + pos) % RINGBUFFER_SIZE]) >> shift) & 1;
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

            // HAMMING checksum
            uint8_t checksum = 0;
            for (k = 0; k < 3; k++) {
                uint8_t masked = tact & tact_hamming_parity_check_matrix[k];
                int l; bool syndrome = 0;
                for (l = 0; l < 7; l++) syndrome ^= (masked >> l) & 1;
                checksum = (checksum << 1) | syndrome;
            }
            // correct single bit error
            if (checksum > 0) tact ^= 1 << (tact_hamming_corrections[checksum] - 1);

            uint8_t slot = (tact & 32) >> 5;
            uint8_t busy = (tact & 64) >> 6;
            uint8_t lcss = (tact & 24) >> 3;

            if (synctype != SYNCTYPE_UNKNOWN && synctypes[slot] != synctype) {
                synctypes[slot] = synctype;
                // send synctype change over metadata fifo
                char metadata[255] = "\n";
                if (synctype == SYNCTYPE_VOICE) {
                    sprintf(metadata, "protocol:DMR;slot:%i;type:voice\n", slot);
                } else if (synctype == SYNCTYPE_DATA) {
                    sprintf(metadata, "protocol:DMR;slot:%i;type:data\n", slot);
                } else {
                    sprintf(metadata, "protocol:DMR\n");
                }
                meta_write(&metadata[0]);
            }

            fprintf(stderr, "  slot: %i busy: %i, lcss: %i", slot, busy, lcss);

            if (emb_present) {
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
                uint8_t deinterleaved[36];
                deinterleave_voice_payload(payload, deinterleaved);
                fwrite(deinterleaved, 1, 36, stdout);
                fflush(stdout);
            }

            // advance to the next frame. as long as we have sync, we know where the next frame begins
            ringbuffer_read_pos = mod(ringbuffer_read_pos + 144, RINGBUFFER_SIZE);
        }
    }
}
