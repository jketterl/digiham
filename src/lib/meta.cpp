#include "meta.hpp"
#include <sstream>
#include <cstring>

using namespace Digiham;

std::string MetaWriter::serializeMetaData(std::map<std::string, std::string> metadata) {
    std::stringstream ss;
    for (auto it = metadata.begin(); it != metadata.end(); it++) {
        ss << it->first << ":" << it->second;
        if (it != metadata.end()) ss << ";";
    }
    ss << "\n";
    return ss.str();
}

FileMetaWriter::FileMetaWriter(FILE* out): file(out) {}

FileMetaWriter::~FileMetaWriter() {
    fclose(file);
}

void FileMetaWriter::sendMetaData(std::map<std::string, std::string> metadata) {
    std::string metaString = serializeMetaData(metadata);
    fwrite(metaString.c_str(), 1, metaString.length(), file);
    fflush(file);
}

void PipelineMetaWriter::sendMetaData(std::map<std::string, std::string> metadata) {
    std::string metaString = serializeMetaData(metadata);
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
    held = true;
}

void MetaCollector::release() {
    held = false;
    sendMetaData();
}

std::map<std::string, std::string> MetaCollector::collect() {
    return std::map<std::string, std::string> { {"protocol", getProtocol()} };
}


void MetaCollector::sendMetaData() {
    if (writer == nullptr || held) return;
    writer->sendMetaData(collect());
}