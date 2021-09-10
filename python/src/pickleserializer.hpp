#pragma once

#include <digiham/meta.hpp>
#include <csdr/source.hpp>

namespace Digiham {

    class PickleSerializer: public Serializer {
        public:
            std::string serializeMetaData(std::map<std::string, std::string> metadata) override;
    };

}