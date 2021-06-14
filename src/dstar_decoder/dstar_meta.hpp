#pragma once

#include "header.hpp"
#include "meta.hpp"
#include <string>

namespace Digiham::DStar {

    class MetaWriter: public Digiham::MetaWriter {
        public:
            void setSync(std::string sync);
            void setHeader(Header* header);
            void setMessage(std::string message);
            void setDPRS(std::string dprs);
            void setGPS(float lat, float lon);
            void reset();
        protected:
            std::string getProtocol() override;
            void sendMetaData() override;
        private:
            Header* header = nullptr;
            std::string sync = "";
            std::string message = "";
            std::string dprs = "";
            float lat;
            float lon;
            bool gpsSet = false;
    };

}