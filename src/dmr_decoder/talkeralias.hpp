#pragma once

#include <string>

#define TALKER_ALIAS_FORMAT_7BIT 0
#define TALKER_ALIAS_FORMAT_8BIT 1
#define TALKER_ALIAS_FORMAT_UTF8 2
#define TALKER_ALIAS_FORMAT_UTF16 3

namespace Digiham::Dmr {

    class TalkerAliasCollector {
        public:
            TalkerAliasCollector();
            ~TalkerAliasCollector();
            void setHeader(unsigned char* data);
            void setBlock(int block, unsigned char* data);
            void reset();
            bool isComplete();
            std::string getContents();
        private:
            unsigned char* data;
            bool headerSeen = false;
            unsigned int blocks = 0;
            unsigned char dataFormat;
            unsigned char length;
    };

}