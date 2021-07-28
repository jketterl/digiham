#pragma once

#include <csdr/module.hpp>

#define VARIANCE_SYMBOLS 100
#define VOLUME_RB_SIZE 100

namespace Digiham {

    namespace FskDemodulator {

        class FskDemodulator: public Csdr::Module<float, unsigned char> {
            public:
                explicit FskDemodulator(unsigned int samplesPerSymbol, bool invert = false);
                ~FskDemodulator() override;
                bool canProcess() override;
                void process() override;
            private:
                void calibrateAudio();
                unsigned int samplesPerSymbol;
                bool invert;
                unsigned int lowestEval;
                unsigned int highestEval;
                unsigned int variance_rb_size;
                unsigned int variance_rb_pos = 0;
                float* variance_rb;
                int variance_offset = 0;
                float volume_rb[VOLUME_RB_SIZE];
                unsigned int volume_rb_pos = 0;
                float min = 0;
                float max = 0;
                float center = 0;
                float umid = 0;
                float lmid = 0;
        };

    }

}