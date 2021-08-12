#pragma once

#include "header.hpp"
#include "meta.hpp"
#include <string>

namespace Digiham::DStar {

    class MetaCollector: public Digiham::MetaCollector {
        public:
            void setSync(std::string sync);
            void setFromHeader(Header* header);
            void setMessage(std::string message);
            void setDeparture(std::string departure);
            void setDestination(std::string destination);
            void setOurCall(std::string ourCall);
            void setYourCall(std::string yourCall);
            void setDPRS(std::string dprs);
            void setGPS(float lat, float lon);
            void reset();
        protected:
            std::string getProtocol() override;
            std::map<std::string, std::string> collect() override;
        private:
            std::string sync;
            std::string message;
            std::string departure;
            std::string destination;
            std::string ourCall;
            std::string yourCall;
            std::string dprs;
            float lat;
            float lon;
            bool gpsSet = false;
    };

}