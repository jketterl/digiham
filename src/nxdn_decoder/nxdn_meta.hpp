#pragma once

#include "meta.hpp"
#include "sacch.hpp"
#include <string>

namespace Digiham::Nxdn {

    class MetaWriter: public Digiham::MetaWriter {
        public:
            void setSync(std::string sync);
            void setSacch(SacchSuperframe* sacch);
            void reset();
        protected:
            void sendMetaData() override;
            std::string getProtocol() override;
        private:
            std::string sync = "";
            SacchSuperframe* sacch = nullptr;
    };

}