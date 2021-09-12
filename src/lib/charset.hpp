#pragma once

#include <string>

namespace Digiham {

    class Converter {
        public:
            static std::string convertToUtf8(const char* input, const size_t length, const char* charset = "iso-8859-1");
    };

}