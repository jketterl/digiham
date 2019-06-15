#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include "version.h"

#define BUF_SIZE 256

/* Filter generated using the online tools available at https://www-users.cs.york.ac.uk/~fisher/mkfilter/racos.html
detailed parameters:

filtertype	=	Raised Cosine
samplerate	=	48000
corner	=	2880
beta	=	0.223
impulselen	=	81
racos	=	sqrt
comp	=	no
bits	=
logmin	=
*/

/* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
   Command line: /www/usr/fisher/helpers/mkshape -r 6.0000000000e-02 2.2300000000e-01 81 -l */

#define NZEROS 80
#define GAIN   8.366778117e+00

static float xv[NZEROS+1];

static float xcoeffs[] =
  { -0.0121655122, -0.0073998323, +0.0000820070, +0.0090314347,
    +0.0176885670, +0.0240832007, +0.0264177785, +0.0234650729,
    +0.0149030392, +0.0015129698, -0.0148146462, -0.0312914078,
    -0.0446456782, -0.0516869182, -0.0499318725, -0.0381834343,
    -0.0169494077, +0.0113952151, +0.0427635909, +0.0718416053,
    +0.0928570445, +0.1005348833, +0.0910983201, +0.0631533365,
    +0.0182979948, -0.0386722777, -0.1000386125, -0.1559171358,
    -0.1954313380, -0.2081578064, -0.1856522409, -0.1228369647,
    -0.0190377028, +0.1215032380, +0.2897427451, +0.4726006031,
    +0.6542974057, +0.8181078829, +0.9483065312, +1.0320490801,
    +1.0609366008, +1.0320490801, +0.9483065312, +0.8181078829,
    +0.6542974057, +0.4726006031, +0.2897427451, +0.1215032380,
    -0.0190377028, -0.1228369647, -0.1856522409, -0.2081578064,
    -0.1954313380, -0.1559171358, -0.1000386125, -0.0386722777,
    +0.0182979948, +0.0631533365, +0.0910983201, +0.1005348833,
    +0.0928570445, +0.0718416053, +0.0427635909, +0.0113952151,
    -0.0169494077, -0.0381834343, -0.0499318725, -0.0516869182,
    -0.0446456782, -0.0312914078, -0.0148146462, +0.0015129698,
    +0.0149030392, +0.0234650729, +0.0264177785, +0.0240832007,
    +0.0176885670, +0.0090314347, +0.0000820070, -0.0073998323,
    -0.0121655122,
  };

float rrc_filter(float sample) {
  float sum; int i;
  
  for (i = 0; i < NZEROS; i++)
      xv[i] = xv[i+1];

  xv[NZEROS] = sample; // unfiltered sample in
  sum = 0.0f;

  for (i = 0; i <= NZEROS; i++)
    sum += (xcoeffs[i] * xv[i]);

  return (sum / GAIN); // filtered sample out
}

float buf[BUF_SIZE];
int r = 0;

void print_usage() {
    fprintf(stderr,
        "rrc_filter version %s\n\n"
        "Usage: rrc_filter [options]\n\n"
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
    while ((r = fread(buf, 4, BUF_SIZE, stdin)) > 0) {
        int i;
        for (i = 0; i < r; i++) {
            buf[i] = rrc_filter(buf[i]);
        }
        fwrite(buf, 4, r, stdout);
        fflush(stdout);
    }

    return 0;
}
