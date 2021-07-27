#pragma once

#include <csdr/module.hpp>

namespace Digiham {

    namespace DcBlock {

        class DcBlock : public Csdr::AnyLengthModule<float, float> {
            public:
                void process(float *input, float *output, size_t length) override;

            private:
                float xm1 = 0.0f;
                float ym1 = 0.0f;
        };

    }

}