#pragma once

#include <cstdio>
#include <map>
#include <string>

namespace Digiham {

    class MetaWriter {
        public:
            MetaWriter(FILE* out);
            MetaWriter();
            ~MetaWriter();
            void setFile(FILE* out);
        protected:
            virtual void sendMetaData() = 0;
            void sendMetaMap(std::map<std::string, std::string> metadata);
            virtual std::string getProtocol() = 0;
            void hold();
            void release();
        private:
            FILE* file = nullptr;
            bool held = false;
    };

}