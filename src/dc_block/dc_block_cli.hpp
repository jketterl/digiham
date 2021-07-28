#pragma once

#include "cli.hpp"

namespace Digiham::DcBlock {

    class Cli: public Digiham::Cli<float> {
        protected:
            std::string getName() override;
            Csdr::Module<float, float>* buildModule() override;
    };

}