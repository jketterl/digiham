#pragma once

#include "meta.hpp"

#include <string>

namespace Digiham::Dmr {

    class MetaCollector: public Digiham::MetaCollector {
        protected:
            std::string getProtocol() override;
    };

}