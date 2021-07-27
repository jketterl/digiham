#pragma once

#include "decoder.hpp"

namespace Digiham {

    template <typename T>
    class Cli {
        public:
            explicit Cli(Csdr::Module<T, T>* decoder);
            virtual ~Cli();
            int main (int argc, char** argv);
        protected:
            virtual std::string getName() = 0;
            virtual void printUsage();
            virtual void printVersion();
            virtual bool parseOptions(int argc, char** argv);
            Csdr::Module<T, T>* decoder;
        private:
            bool read();
            Csdr::Ringbuffer<T>* ringbuffer;
    };

}