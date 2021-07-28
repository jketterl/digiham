#include <cfloat>
#include "fsk_demodulator.hpp"

using namespace Digiham::FskDemodulator;

FskDemodulator::FskDemodulator(unsigned int samplesPerSymbol, bool invert):
    samplesPerSymbol(samplesPerSymbol),
    invert(invert),
    lowestEval((int) roundf((float)samplesPerSymbol / 3)),
    highestEval((int) roundf((float)samplesPerSymbol * 2 / 3)),
    variance_rb_size(VARIANCE_SYMBOLS * samplesPerSymbol),
    variance_rb((float*) malloc(sizeof(float) * variance_rb_size))
{}

FskDemodulator::~FskDemodulator() {
    free(variance_rb);
}

bool FskDemodulator::canProcess() {
    return reader->available() > samplesPerSymbol + 1 && writer->writeable() > 0;
}

void FskDemodulator::process() {
    float* input = reader->getReadPointer() + variance_offset;
    unsigned char* output = writer->getWritePointer();
    // reset until next vairance evaluation
    variance_offset = 0;

    float sum = 0;
    float volume_sum = 0;
    for (size_t i = 0; i < samplesPerSymbol; i++) {
        float value = input[i];
        if (i >= lowestEval && i < highestEval) sum += value;
        volume_sum += value;
        variance_rb[variance_rb_pos + i] = value;
    }
    reader->advance(samplesPerSymbol);

    variance_rb_pos += samplesPerSymbol;
    if (variance_rb_pos >= variance_rb_size) {

        double vmin;
        size_t vmin_pos;

        for (size_t i = 0; i < samplesPerSymbol; i++) {
            //fprintf(stderr, "variance calc @ %i: ", i);
            float total = 0;
            int k;
            for (k = 0; k < VARIANCE_SYMBOLS; k++) {
                total += variance_rb[k * samplesPerSymbol + i];
            }
            double mean = total / VARIANCE_SYMBOLS;
            //fprintf(stderr, "total: %i, mean: %.0f ", total, mean);

            double dsum = 0;
            for (k = 0; k < VARIANCE_SYMBOLS; k++) {
                dsum += pow(mean - variance_rb[k * samplesPerSymbol + i], 2);
            }
            double variance = dsum / VARIANCE_SYMBOLS;
            //fprintf(stderr, "variance: %.0f\n", variance);

            if (i == 0 || variance < vmin) {
                vmin = variance;
                vmin_pos = i;
            }
        }

        if (vmin <= 0 || vmin > 5000000) {
            // NOOP
        } else if (vmin_pos > 0 && vmin_pos < samplesPerSymbol / 2) {
            // variance indicates stepping to the left
            variance_offset = 1;
        } else if (vmin_pos >= samplesPerSymbol / 2 && vmin_pos < samplesPerSymbol - 1) {
            // variance indicates stepping to the right
            variance_offset = -1;
        }

        variance_rb_pos %= variance_rb_size;
    }

    float volume_average = volume_sum / samplesPerSymbol;
    volume_rb[volume_rb_pos] = volume_average;

    volume_rb_pos += 1;
    if (volume_rb_pos >= VOLUME_RB_SIZE) volume_rb_pos = 0;

    calibrateAudio();

    float average = sum / (highestEval - lowestEval);

    if (average > center) {
        *output = !invert;
    } else {
        *output = invert;
    }

    writer->advance(1);
}

void FskDemodulator::calibrateAudio() {
    int i;
    min = FLT_MAX, max = FLT_MIN;

    for (i = 0; i < VOLUME_RB_SIZE; i++) {
        if (volume_rb[i] < min) min = volume_rb[i];
        if (volume_rb[i] > max) max = volume_rb[i];
    }

    center = (max + min) / 2;

    umid = ((max - center) * 0.625) + center;
    lmid = ((min - center) * 0.625) + center;
}