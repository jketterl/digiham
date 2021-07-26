#pragma once

#include "cli.hpp"

namespace Digiham::Nxdn {

    class Cli: public Digiham::Cli {
        public:
            Cli();
            std::string getName() override;
    };

}