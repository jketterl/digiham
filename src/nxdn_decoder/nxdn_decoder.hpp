#pragma once

#include "decoder.hpp"

namespace Digiham::Nxdn {

    class Decoder: public Digiham::Decoder {
        public:
            Decoder();
        protected:
            std::string getName() override;
            Phase* getInitialPhase() override;
    };

}