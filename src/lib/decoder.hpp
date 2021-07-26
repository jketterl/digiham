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
            bool canProcess() override;
            void process() override;
            int main (int argc, char** argv);
            Decoder(MetaWriter* meta);
            ~Decoder();
        protected:
            virtual std::string getName() = 0;
            virtual void printUsage();
            virtual void printVersion();
            virtual bool parseOptions(int argc, char** argv);
            virtual Phase* getInitialPhase() = 0;
        private:
            bool read();
            void setPhase(Phase* phase);
            MetaWriter* meta;
            Csdr::Ringbuffer<unsigned char>* ringbuffer;
            Phase* currentPhase = nullptr;
    };

}