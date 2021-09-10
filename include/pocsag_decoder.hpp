#pragma once

#include "decoder.hpp"

namespace Digiham {

    namespace Pocsag {

        class Decoder: public Digiham::Decoder {
            public:
                Decoder();
                explicit Decoder(Serializer* serializer);
            protected:
                virtual void setPhase(Digiham::Phase* phase) override;
                Serializer* serializer;
        };

    }

}