#pragma once

#include "meta.hpp"

namespace Digiham::Nxdn {

    class MetaWriter: public Digiham::MetaWriter {
        protected:
            void sendMetaData() override;
            std::string getProtocol() override;
    };

}