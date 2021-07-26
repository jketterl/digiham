#pragma once

#include "meta.hpp"
#include <csdr/reader.hpp>
#include <csdr/writer.hpp>

namespace Digiham {

    class Phase {
        public:
            virtual ~Phase() = default;
            virtual int getRequiredData() = 0;
            virtual Phase* process(Csdr::Reader<unsigned char>* data, Csdr::Writer<unsigned char>* output) = 0;
            void setMetaWriter(MetaWriter* meta);
        protected:
            MetaWriter* meta;
    };

}