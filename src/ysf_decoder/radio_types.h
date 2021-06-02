#include <stdint.h>

struct radio {
    uint8_t id;
    char* name;
};

static struct radio radio_types[] = {
    {0x20, "DR-2X"},
    {0x24, "FT-1D"},
    {0x25, "FTM-400D"},
    {0x26, "DR-1X"},
    {0x27, "FT-991"},
    {0x28, "FT-2D"},
    {0x29, "FTM-100D"},
    {0x2B, "FT-70D"},
    {0x30, "FT-3D"},
    {0x31, "FTM-300D"}
};

char* get_radio_type(uint8_t id);