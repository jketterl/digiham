#pragma once

#include "header.hpp"
#include "meta.hpp"
#include <string>

namespace Digiham::DStar {

    class MetaCollector: public Digiham::MetaCollector {
        public:
            void setSync(std::string sync);
            void setHeader(Header* header);
            void setMessage(std::string message);
            void setDPRS(std::string dprs);
            void setGPS(float lat, float lon);
            void reset();
        protected:
            std::string getProtocol() override;
            std::map<std::string, std::string> collect() override;
        private:
            Header* header = nullptr;
            std::string sync;
            std::string message;
            std::string dprs;
            float lat;
            float lon;
            bool gpsSet = false;
    };

}