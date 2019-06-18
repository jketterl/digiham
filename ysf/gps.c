#include "gps.h"

bool decode_gps(uint8_t* data, coordinate* result) {
    if ((data[0] & 0x0F) > 9) return false;
    if ((data[1] & 0x0F) > 9) return false;
    if ((data[2] & 0x0F) > 9) return false;
    if ((data[3] & 0x0F) > 9) return false;
    if ((data[4] & 0x0F) > 9) return false;
    if ((data[5] & 0x0F) > 9) return false;

    float lat = (data[0] & 0x0F) * 10 +
                (data[1] & 0x0F) +
        (float) (data[2] & 0x0F) / 6 +
        (float) (data[3] & 0x0F) / 60 +
        (float) (data[4] & 0x0F) / 600 +
        (float) (data[5] & 0x0F) / 6000;

    uint8_t direction = (data[3] & 0xF0);
    if (direction == 0x50) {
        // northern hemisphere. nothing to do here, so pass
    } else if (direction == 0x30) {
        // southern hemisphere
        lat *= -1;
    } else {
        return false;
    }

    float lon;

    uint8_t b = data[4] & 0xF0;
    uint8_t c = data[6];
    if (b == 0x50) {
        if (c >= 0x76 && c < 0x7f) {
            lon = c - 0x76;
        } else if (c >= 0x6c && c < 0x75) {
            lon = 100 + (c - 0x6c);
        } else if (c >= 0x26 && c < 0x6b) {
            lon = 110 + (c - 0x26);
        } else {
            return false;
        }
    } else if (b == 0x30) {
        if (c >= 0x26 && c < 0x7f) {
            lon = 10 + (c - 0x26);
        } else {
            return false;
        }
    }

    b = data[7];
    if (b > 0x58 && b <= 0x61) {
        lon += (float) (b - 0x58) / 60;
    } else if (b >= 0x26 && b <= 0x57) {
        lon += (float) (10 + (b - 0x26)) / 60;
    } else {
        return false;
    }

    b = data[8];
    if (b >= 0x1c && b < 0x7f) {
        lon += (float) (b - 0x1c) / 6000;
    } else {
        return false;
    }

    direction = (data[5] & 0xF0);
    if (direction == 0x50) {
        // western hemisphere
        lon *= -1;
    } else if (direction == 0x30) {
        // eastern hemisphere. nothing to do here, so pass
    } else {
        return false;
    }



    if (lat > 90 || lat < -90) return false;
    if (lon > 180 || lon < -180) return false;

    result->lat = lat;
    result->lon = lon;

    return true;
}
