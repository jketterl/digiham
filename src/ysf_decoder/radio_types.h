#include <stdint.h>

struct radio {
    uint8_t id;
    char* name;
};

static struct radio radio_types[] = {
    {0x20, (char*) "DR-2X"},
    {0x24, (char*) "FT-1D"},
    {0x25, (char*) "FTM-400D"},
    {0x26, (char*) "DR-1X"},
    {0x27, (char*) "FT-991"},
    {0x28, (char*) "FT-2D"},
    {0x29, (char*) "FTM-100D"},
    {0x2B, (char*) "FT-70D"},
    {0x30, (char*) "FT-3D"},
    {0x31, (char*) "FTM-300D"}
};

char* get_radio_type(uint8_t id);