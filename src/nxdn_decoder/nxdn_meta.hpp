#pragma once

#include "meta.hpp"
#include "sacch.hpp"
#include <string>

namespace Digiham::Nxdn {

    class MetaCollector: public Digiham::MetaCollector {
        public:
            void setSync(std::string sync);
            void setSacch(SacchSuperframe* sacch);
            void reset();
        protected:
            std::map<std::string, std::string> collect() override;
            std::string getProtocol() override;
        private:
            std::string sync;
            SacchSuperframe* sacch = nullptr;
    };

}