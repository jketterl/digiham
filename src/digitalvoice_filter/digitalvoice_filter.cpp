#include <climits>
#include "digitalvoice_filter.hpp"

using namespace Digiham::DigitalVoice;

void DigitalVoiceFilter::process(short *input, short *output, size_t length) {
    for (size_t i = 0; i < length; i++) {
        output[i] = (short) (filter((float)input[i] / SHRT_MAX) * SHRT_MAX);
    }
}

/*
filter parameter:

filtertype	=	Butterworth
passtype	=	Bandpass
ripple	=
order	=	5
samplerate	=	8000
corner1	=	200
corner2	=	3400
adzero	=
logmin	=
*/

/* Digital filter designed by mkfilter/mkshape/gencode   A.J. Fisher
   Command line: /www/usr/fisher/helpers/mkfilter -Bu -Bp -o 5 -a 2.5000000000e-02 4.2500000000e-01 -l */

// this is what mkfilter gives us, but i have experienced clipping based on this
//#define GAIN   2.823549227e+00
// this seems to work, but is purely based on experimentation
# define GAIN 5

float DigitalVoiceFilter::filter(float sample) {
    xv[0] = xv[1]; xv[1] = xv[2]; xv[2] = xv[3]; xv[3] = xv[4]; xv[4] = xv[5]; xv[5] = xv[6]; xv[6] = xv[7]; xv[7] = xv[8]; xv[8] = xv[9]; xv[9] = xv[10];
    xv[10] = sample / GAIN;
    yv[0] = yv[1]; yv[1] = yv[2]; yv[2] = yv[3]; yv[3] = yv[4]; yv[4] = yv[5]; yv[5] = yv[6]; yv[6] = yv[7]; yv[7] = yv[8]; yv[8] = yv[9]; yv[9] = yv[10];
    yv[10] =   (xv[10] - xv[0]) + 5 * (xv[2] - xv[8]) + 10 * (xv[6] - xv[4])
               + (  0.1254306222 * yv[0]) + (  0.1285714097 * yv[1])
               + ( -0.8106454980 * yv[2]) + ( -0.7664515771 * yv[3])
               + (  2.1846187758 * yv[4]) + (  1.8106678608 * yv[5])
               + ( -3.1465011600 * yv[6]) + ( -2.0391991609 * yv[7])
               + (  2.4873968618 * yv[8]) + (  1.0249072542 * yv[9]);
    return yv[10];
}