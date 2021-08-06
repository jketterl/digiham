#pragma once

#include "decoder.hpp"

namespace Digiham {

    namespace Dmr {

        class Decoder: public Digiham::Decoder {
            public:
                Decoder();
                void setSlotFilter(unsigned char filter);
            protected:
                void setPhase(Digiham::Phase* phase) override;
            private:
                unsigned char slotFilter = 3;
        };

    }

}