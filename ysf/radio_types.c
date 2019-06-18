#include "radio_types.h"

char* get_radio_type(uint8_t id) {
    int num_radios = sizeof(radio_types)/sizeof(radio_types[0]);
    for (int i = 0; i < num_radios; i++) {
        if (radio_types[i].id == id) return radio_types[i].name;
    }
    return "Unknown radio";
}