#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include "version.h"

#define BUF_SIZE 256
float buf[BUF_SIZE];
int r = 0;
#define R 0.995f
#define GAIN (1 + R) / 2

void print_usage() {
    fprintf(stderr,
        "dc_block version %s\n\n"
        "Usage: dc_block [options]\n\n"
        "Available options:\n"
        " -h, --help      show this message\n"
        " -v, --version   print version and exit\n",
        VERSION
    );
}

int main(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"version", no_argument, NULL, 'v'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
        switch (c) {
            case 'v':
                print_version();
                return 0;
            case 'h':
                print_usage();
                return 0;
        }
    }

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
        fflush(stdout);
    }

    return 0;
}
