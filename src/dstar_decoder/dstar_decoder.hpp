#pragma once

#include "decoder.hpp"
#include "phase.hpp"
#include <string>

namespace Digiham::DStar {

    class Decoder: public Digiham::Decoder {
        public:
            Decoder();
    };

    class Cli: public Digiham::Cli {
        public:
            Cli();
            std::string getName() override;
    };

}