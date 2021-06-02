#!/usr/bin/env bash
set -euo pipefail

# This is an example Pocsag decoding pipeline to show the basic usage of the tools in this project.
# The individual steps are documented below.

if [ $# -eq 0 ]; then
    echo "Usage: $0 frequency"
    exit 1
fi

# this gives us an narrow-fm demodulated audio signal at 48kHz
rtl_fm -f $1 -M fm -s 48000 | \
# the toolchain needs 32bit float input, so we need to convert it
csdr convert_s16_f | \
# decode the audio and get the raw bitsream from the signal
# inverted
# FSK with 1200 baud, so 40 samples per symbol
fsk_demodulator -i -s 40 | \
# decode pocsag messages
pocsag_decoder