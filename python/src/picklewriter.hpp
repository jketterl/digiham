#pragma once

#include <digiham/meta.hpp>
#include <csdr/source.hpp>

namespace Digiham {

    class PickleWriter: public MetaWriter, public Csdr::Source<unsigned char>{
        public:
            void sendMetaData(std::map<std::string, std::string> metadata) override;
    };

}