#!/usr/bin/env bash
set -euo pipefail

# This is an example YSF decoding pipeline to show the basic usage of the tools in this project.
# The individual steps are documented below.

if [ $# -eq 0 ]; then
    echo "Usage: $0 frequency"
    exit 1
fi

# this gives us an narrow-fm demodulated audio signal at 48kHz
rtl_fm -f $1 -M fm -s 48000 | \
# the toolchain needs 32bit float input, so we need to convert it
csdr convert -i s16 -o float | \
# block out any dc offset that may be present due to oscillator offset
csdr dcblock | \
# raised root cosine filter as specified by the DMR spec
rrc_filter | \
# decode the audio and get the raw bitsream from the signal
gfsk_demodulator | \
# this implements the YSF protocol layer
ysf_decoder | \
# decode the MBE voice codec
mbe_synthesizer --yaesu | \
# filter out unwanted audio frequencies
digitalvoice_filter | \
# play the result through the nearest soundcard
play -t raw -e s -c 1 -b 16 -r 8000 -