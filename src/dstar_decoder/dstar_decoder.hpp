#pragma once

#include "decoder.hpp"
#include "phase.hpp"
#include <string>

namespace Digiham::DStar {

    class Decoder: public Digiham::Decoder {
        public:
            Decoder();
        protected:
            std::string getName() override;
            Phase* getInitialPhase() override;
    };

}