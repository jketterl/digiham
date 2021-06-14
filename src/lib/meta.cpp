#include "meta.hpp"
#include <sstream>

using namespace Digiham;

MetaWriter::MetaWriter(FILE* out): file(out) {}

MetaWriter::MetaWriter(): MetaWriter(nullptr) {}

MetaWriter::~MetaWriter() {
    if (file != nullptr) {
        fclose(file);
    }
}

void MetaWriter::setFile(FILE* out) {
    file = out;
}

void MetaWriter::hold() {
    held = true;
}

void MetaWriter::release() {
    held = false;
    sendMetaData();
}

void MetaWriter::sendMetaMap(std::map<std::string, std::string> metadata) {
    std::stringstream ss;
    ss << "protocol:" << getProtocol();
    for (std::map<std::string, std::string>::iterator it = metadata.begin(); it != metadata.end(); it++) {
        ss << ";" << it->first << ":" << it->second;
    }
    ss << "\n";

    std::string metaString = ss.str();
    if (held || file == nullptr) {
        return;
    }
    fwrite(metaString.c_str(), 1, metaString.length(), file);
    fflush(file);
}