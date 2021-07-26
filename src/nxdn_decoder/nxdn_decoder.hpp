#pragma once

#include "decoder.hpp"

namespace Digiham::Nxdn {

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