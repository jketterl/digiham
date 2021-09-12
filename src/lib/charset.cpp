#include "charset.hpp"

#include <unicode/ucnv.h>
#include <cassert>

#include <iostream>

using namespace Digiham;

std::string Converter::convertToUtf8(const char* input, const size_t length, const char* charset) {
    // sanity check...
    if (length == 0) return "";

    UErrorCode err = U_ZERO_ERROR;
    // overallocate since i don't know how to predict the resulting length
    size_t targetLength = (length + 1) * 2;
    auto target = (char*) calloc(sizeof(char), targetLength);
    ucnv_convert("utf-8", charset, target, targetLength, input, length, &err);
    std::string result;
    if (!U_SUCCESS(err)) {
        std::cerr << "ucnv error: " << u_errorName(err) << "\n";
    } else {
        result = std::string(target);
    }
    free(target);
    return result;
}