#pragma once

#include <string>
#include <csdr/ringbuffer.hpp>
#include <csdr/module.hpp>

#include "meta.hpp"

#define BUF_SIZE 128
#define RINGBUFFER_SIZE 1024

namespace Digiham {

    // private API
    class Phase;

    class Decoder: public Csdr::Module<unsigned char, unsigned char> {
        public:
            Decoder(Phase* initialPhase, MetaCollector* collector);
            ~Decoder() override;
            bool canProcess() override;
            void process() override;
            void setMetaWriter(MetaWriter* meta);
        private:
            void setPhase(Phase* phase);
            MetaCollector* metaCollector;
            Phase* currentPhase;
    };

}