#pragma once

#include "decoder.hpp"
#include <sstream>
#include <vector>
#include <getopt.h>

namespace Digiham {

    template <typename T, typename U>
    class Cli {
        public:
            Cli();
            virtual ~Cli();
            int main (int argc, char** argv);
        protected:
            virtual std::string getName() = 0;
            virtual std::stringstream getUsageString();
            virtual std::vector<struct option> getOptions();
            virtual bool receiveOption(int c, char* optarg);
            virtual void printVersion();
            virtual bool parseOptions(int argc, char** argv);
            virtual Csdr::Module<T, U>* buildModule() = 0;
        private:
            bool read();
            Csdr::Ringbuffer<T>* ringbuffer;
    };

    class DecoderCli: public Cli<unsigned char, unsigned char> {
        protected:
            Decoder* buildModule() override = 0;
            std::stringstream getUsageString() override;
            std::vector<struct option> getOptions() override;
            bool receiveOption(int c, char* optarg) override;
            FILE* metaFile = nullptr;
    };

}