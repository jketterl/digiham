#pragma once

#include "cli.hpp"

namespace Digiham::DStar {

    class Cli: public Digiham::DecoderCli {
        protected:
            std::string getName() override;
            Decoder* buildModule() override;
    };

}