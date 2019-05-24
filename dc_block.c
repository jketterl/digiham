#include <unistd.h>
#include <stdio.h>

#define BUF_SIZE 256
float buf[BUF_SIZE];
int r = 0;
#define R 0.995f
#define GAIN (1 + R) / 2

int main() {
    float xm1 = 0;
    float ym1 = 0;
    while ((r = fread(buf, 4, BUF_SIZE, stdin)) > 0) {
        int i;
        for (i = 0; i < r; i++) {
            // dc block filter implementation according to https://www.dsprelated.com/freebooks/filters/DC_Blocker.html
            float x = buf[i];
            float y = GAIN * (x - xm1) + R * ym1;
            xm1 = x;
            ym1 = y;
            buf[i] = y;
        }
        fwrite(buf, 4, r, stdout);
    }

    return 0;
}
