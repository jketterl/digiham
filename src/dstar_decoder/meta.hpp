#pragma once

#include "header.hpp"
#include <cstdio>
#include <map>
#include <string>

namespace Digiham::DStar {

    class MetaWriter {
        public:
            MetaWriter(FILE* out);
            MetaWriter();
            ~MetaWriter();
            void setFile(FILE* out);
            void setSync(std::string sync);
            void setHeader(Header* header);
            void setMessage(std::string message);
            void setDPRS(std::string dprs);
            void reset();
            void hold();
            void release();
        private:
            void sendMetaData();
            Header* header = nullptr;
            std::string sync = "";
            std::string message = "";
            std::string dprs = "";
            FILE* file = nullptr;
            bool held = false;
    };

}