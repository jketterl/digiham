#pragma once

#include "meta.hpp"
#include "phase.hpp"
#include <string>
#include <csdr/ringbuffer.hpp>
#include <csdr/module.hpp>

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

namespace Digiham {

    class Decoder: public Csdr::Module<unsigned char, unsigned char> {
        public:
            Decoder(MetaWriter* meta, Phase* initialPhase);
            ~Decoder();
            bool canProcess() override;
            void process() override;
            void setMetaFile(FILE* file);
        private:
            void setPhase(Phase* phase);
            MetaWriter* meta;
            Phase* currentPhase = nullptr;
    };

    class Cli {
        public:
            Cli(Decoder* decoder);
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