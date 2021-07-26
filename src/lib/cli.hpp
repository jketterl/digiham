#pragma once

#include "decoder.hpp"

namespace Digiham {

    class Cli {
        public:
            explicit Cli(Decoder* decoder);
            virtual ~Cli();
            int main (int argc, char** argv);
        protected:
            virtual std::string getName() = 0;
            virtual void printUsage();
            virtual void printVersion();
            virtual bool parseOptions(int argc, char** argv);
            Decoder* decoder;
        private:
            bool read();
            Csdr::Ringbuffer<unsigned char>* ringbuffer;
    };

}