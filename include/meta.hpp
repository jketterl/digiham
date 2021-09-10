#pragma once

#include <cstdio>
#include <map>
#include <string>
#include <csdr/source.hpp>

namespace Digiham {

    class Serializer {
        public:
            virtual ~Serializer() = default;
            virtual std::string serializeMetaData(std::map<std::string, std::string> metadata) = 0;
    };

    class StringSerializer: public Serializer {
        public:
            std::string serializeMetaData(std::map<std::string, std::string> metadata) override;
    };

    class MetaWriter {
        public:
            MetaWriter();
            explicit MetaWriter(Serializer* serializer);
            virtual ~MetaWriter();
            virtual void sendMetaData(std::map<std::string, std::string> metadata) = 0;
            void setSerializer(Serializer* serializer);
        protected:
            Serializer* serializer;
    };

    class FileMetaWriter: public MetaWriter {
        public:
            explicit FileMetaWriter(FILE* out);
            FileMetaWriter(FILE* out, Serializer* serializer);
            ~FileMetaWriter() override;
            void sendMetaData(std::map<std::string, std::string> metadata) override;
        private:
            FILE* file = nullptr;
    };

    class PipelineMetaWriter: public MetaWriter, public Csdr::Source<unsigned char> {
        public:
            explicit PipelineMetaWriter(Serializer* serializer);
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