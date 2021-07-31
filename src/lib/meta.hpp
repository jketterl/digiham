#pragma once

#include <cstdio>
#include <map>
#include <string>

namespace Digiham {

    class MetaWriter {
        public:
            virtual ~MetaWriter() = default;
            virtual void sendMetaData(std::map<std::string, std::string> metadata) = 0;
    };

    class FileMetaWriter: public MetaWriter {
        public:
            explicit FileMetaWriter(FILE* out);
            ~FileMetaWriter() override;
        protected:
            void sendMetaData(std::map<std::string, std::string> metadata) override;
        private:
            FILE* file = nullptr;
    };

    class MetaCollector {
        public:
            MetaCollector();
            explicit MetaCollector(MetaWriter* writer);
            virtual ~MetaCollector();
            void setWriter(MetaWriter* writer);
        protected:
            virtual std::string getProtocol() = 0;
            virtual std::map<std::string, std::string> collect();
            void sendMetaData();
            void hold();
            void release();
        private:
            bool held = false;
            MetaWriter* writer;
    };

}