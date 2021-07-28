#pragma once

#include "cli.hpp"

namespace Digiham::FskDemodulator {

    class Cli: public Digiham::Cli<float> {
        protected:
            std::string getName() override;
            Csdr::Module<float, float>* buildModule() override;
            std::stringstream getUsageString() override;
            std::vector<struct option> getOptions() override;
            bool receiveOption(int c, char* optarg) override;
        private:
            unsigned int samplesPerSymbol = 40;
            bool invert = false;
    };

}