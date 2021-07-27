#pragma once

#include "cli.hpp"

namespace Digiham::DcBlock {

    class Cli: public Digiham::Cli<float> {
        public:
            Cli();
            std::string getName() override;
    };

}