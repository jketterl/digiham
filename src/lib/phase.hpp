#pragma once

#include "meta.hpp"
#include "ringbuffer.hpp"

namespace Digiham {

    class Phase {
        public:
            virtual int getRequiredData() = 0;
            virtual Phase* process(Ringbuffer* data, size_t& read_pos) = 0;
            void setMetaWriter(MetaWriter* meta);
        protected:
            MetaWriter* meta;
    };

}