#pragma once

#include <string>
#include <csdr/ringbuffer.hpp>
#include <csdr/module.hpp>

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

namespace Digiham {

    // private API
    class Phase;

    // private API
    class MetaWriter;

    class Decoder: public Csdr::Module<unsigned char, unsigned char> {
        public:
            Decoder(MetaWriter* meta, Phase* initialPhase);
            ~Decoder() override;
            bool canProcess() override;
            void process() override;
            void setMetaFile(FILE* file);
        private:
            void setPhase(Phase* phase);
            MetaWriter* meta;
            Phase* currentPhase = nullptr;
    };

}