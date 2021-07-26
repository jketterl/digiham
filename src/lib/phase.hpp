#pragma once

#include "meta.hpp"
#include <csdr/reader.hpp>

namespace Digiham {

    class Phase {
        public:
            virtual int getRequiredData() = 0;
            virtual Phase* process(Csdr::Reader<unsigned char>* data) = 0;
            void setMetaWriter(MetaWriter* meta);
        protected:
            MetaWriter* meta;
    };

}