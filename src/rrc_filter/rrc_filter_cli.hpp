#pragma once

#include "cli.hpp"

namespace Digiham::RrcFilter {

    class Cli: public Digiham::Cli<float, float> {
        protected:
            std::string getName() override;
            Csdr::Module<float, float>* buildModule() override;
            std::stringstream getUsageString() override;
            std::vector<struct option> getOptions() override;
            bool receiveOption(int c, char* optarg) override;
        private:
            bool narrow = false;
    };

}