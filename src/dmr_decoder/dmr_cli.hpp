#pragma once

#include "cli.hpp"
#include "dmr_decoder.hpp"

#include <thread>

namespace Digiham::Dmr {

    class Cli: public Digiham::DecoderCli {
        public:
            ~Cli() override;
        protected:
            std::string getName() override;
            Decoder* buildModule() override;
            std::stringstream getUsageString() override;
            std::vector<struct option> getOptions() override;
            bool receiveOption(int c, char* optarg) override;
        private:
            Decoder* decoder;
            std::thread* fifoReader = nullptr;
            FILE* fifo = nullptr;
            void fifoLoop();
    };

}