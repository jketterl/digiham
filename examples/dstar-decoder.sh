#!/usr/bin/env bash
set -euo pipefail

# This is an example DMR decoding pipeline to show the basic usage of the tools in this project.
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
# decode the audio and get the raw bitsream from the signal
fsk_demodulator -s 10 | \
# this implements the D-Star protocol layer
dstar_decoder | \
# decode the MBE voice codec
mbe_synthesizer -d | \
# filter out unwanted audio frequencies
digitalvoice_filter | \
# play the result through the nearest soundcard
play -t raw -e s -c 1 -b 16 -r 8000 -