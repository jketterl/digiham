#include "meta.hpp"
#include <sstream>
#include <cstring>
#include <utility>

using namespace Digiham;

std::string StringSerializer::serializeMetaData(std::map<std::string, std::string> metadata) {
    std::stringstream ss;
    for (auto it = metadata.begin(); it != metadata.end(); it++) {
        if (it != metadata.begin()) ss << ";";
        ss << it->first << ":" << it->second;
    }
    ss << "\n";

    return ss.str();
}

MetaWriter::MetaWriter(Serializer *serializer): serializer(serializer) {}

MetaWriter::MetaWriter(): MetaWriter(new StringSerializer()) {}

MetaWriter::~MetaWriter() {
    delete serializer;
}

void MetaWriter::setSerializer(Serializer *serializer) {
    if (serializer == this->serializer) return;
    auto old = this->serializer;
    this->serializer = serializer;
    delete old;
}

FileMetaWriter::FileMetaWriter(FILE* out): MetaWriter(), file(out) {}

FileMetaWriter::FileMetaWriter(FILE *out, Serializer *serializer): MetaWriter(serializer), file(out) {}

FileMetaWriter::~FileMetaWriter() {
    fclose(file);
}

void FileMetaWriter::sendMetaData(std::map<std::string, std::string> metadata) {
    std::string metaString = serializer->serializeMetaData(metadata);
    fwrite(metaString.c_str(), 1, metaString.length(), file);
    fflush(file);
}

PipelineMetaWriter::PipelineMetaWriter(Serializer *serializer): MetaWriter(serializer) {}

void PipelineMetaWriter::sendMetaData(std::map<std::string, std::string> metadata) {
    std::string metaString = serializer->serializeMetaData(metadata);
    // can't write...
    if (writer->writeable() < metaString.length()) return;
    std::memcpy(writer->getWritePointer(), metaString.c_str(), metaString.length());
    writer->advance(metaString.length());
}

MetaCollector::MetaCollector(MetaWriter *writer): writer(writer) {}

MetaCollector::MetaCollector(): MetaCollector(nullptr) {}

MetaCollector::~MetaCollector() {
    delete writer;
}

void MetaCollector::setWriter(MetaWriter *writer) {
    delete this->writer;
    this->writer = writer;
}

void MetaCollector::hold() {
    held++;
}

void MetaCollector::release() {
    held--;
    if (held == 0) {
        if (dirty) sendMetaData();
        dirty = false;
    }
}

std::map<std::string, std::string> MetaCollector::collect() {
    return std::map<std::string, std::string> { {"protocol", getProtocol()} };
}

void MetaCollector::sendMetaData(std::map<std::string, std::string> metadata) {
    if (writer == nullptr) return;
    writer->sendMetaData(std::move(metadata));
}


void MetaCollector::sendMetaData() {
    if (writer == nullptr) return;
    if (held) {
        dirty = true;
        return;
    }
    sendMetaData(collect());
}