#include <stdint.h>

typedef struct {
    uint8_t frame_type;
    uint8_t callsign_type;
    uint8_t call_type;
    uint8_t block_number;
    uint8_t block_total;
    uint8_t frame_number;
    uint8_t frame_total;
    uint8_t frequency_deviation;
    uint8_t message_path;
    uint8_t voip_path;
    uint8_t data_type;
    uint8_t sql_type;
    uint8_t sql_code;
} fich;

fich decode_fich(uint32_t input);
