#pragma once

#include "meta.hpp"
#include "sacch.hpp"
#include <string>

namespace Digiham::Nxdn {

    class MetaCollector: public Digiham::MetaCollector {
        public:
            void setSync(std::string sync);
            void setFromSacch(SacchSuperframe* sacch);
            void setType(std::string type);
            void setSource(uint16_t source);
            void setDestination(uint16_t destination);
            void reset();
        protected:
            std::map<std::string, std::string> collect() override;
            std::string getProtocol() override;
        private:
            std::string sync;
            std::string type;
            uint16_t source = 0;
            uint16_t destination = 0;
    };

}