#pragma once

#include "cli.hpp"

namespace Digiham::DStar {

    class Cli: public Digiham::Cli {
        public:
            Cli();
            std::string getName() override;
    };

}