#pragma once

#include <cstdio>
#include <map>
#include <string>
#include <csdr/source.hpp>

namespace Digiham {

    class MetaWriter {
        public:
            virtual ~MetaWriter() = default;
            virtual void sendMetaData(std::map<std::string, std::string> metadata) = 0;
        protected:
            std::string serializeMetaData(std::map<std::string, std::string> metadata);
    };

    class FileMetaWriter: public MetaWriter {
        public:
            explicit FileMetaWriter(FILE* out);
            ~FileMetaWriter() override;
            void sendMetaData(std::map<std::string, std::string> metadata) override;
        private:
            FILE* file = nullptr;
    };

    class PipelineMetaWriter: public MetaWriter, public Csdr::Source<unsigned char> {
        public:
            void sendMetaData(std::map<std::string, std::string> metadata) override;
    };


    class MetaCollector {
        public:
            MetaCollector();
            explicit MetaCollector(MetaWriter* writer);
            virtual ~MetaCollector();
            void setWriter(MetaWriter* writer);
            void hold();
            void release();
        protected:
            virtual std::string getProtocol() = 0;
            virtual std::map<std::string, std::string> collect();
            virtual void sendMetaData();
            virtual void sendMetaData(std::map<std::string, std::string> metadata);
        private:
            int held = 0;
            bool dirty = false;
            MetaWriter* writer;
    };

}