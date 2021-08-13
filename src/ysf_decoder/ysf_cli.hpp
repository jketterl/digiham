#pragma once

#include "cli.hpp"
#include "ysf_decoder.hpp"

namespace Digiham::Ysf {

    class Cli: public Digiham::DecoderCli {
        protected:
            std::string getName() override;
            Decoder* buildModule() override;
    };

}