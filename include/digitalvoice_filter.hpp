#pragma once

#include <csdr/module.hpp>

#define NZEROS 10
#define NPOLES 10

namespace Digiham{

    namespace DigitalVoice {

        class DigitalVoiceFilter: public Csdr::AnyLengthModule<short, short> {
            public:
                void process(short* input, short* output, size_t length) override;
            private:
                float filter(float sample);
                float xv[NZEROS+1] = {0.0f};
                float yv[NPOLES+1] = {0.0f};
        };

    }

}