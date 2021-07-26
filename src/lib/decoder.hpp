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