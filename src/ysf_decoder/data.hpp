#pragma once

#include <string>

#include "coordinate.hpp"

#define COMMAND_NULL0_GPS     0x22615f
#define COMMAND_SHORT_GPS     0x22625f
#define COMMAND_NULL1_GPS     0x47635f
#define COMMAND_LONG_GPS      0x47645f

namespace Digiham::Ysf {
    class DataFrame {
        public:
            explicit DataFrame(unsigned char* data);
            ~DataFrame();
            uint32_t getCommand();
            Coordinate* getGpsCoordinate();
            std::string getRadio();
        private:
            unsigned char* data;
    };

    class DataCollector {
        public:
            DataCollector();
            ~DataCollector();
            void reset();
            void collect(unsigned char* data, unsigned char offset);
            bool hasCollected(unsigned char num);
            DataFrame* getDataFrame();
        private:
            unsigned char* data;
            unsigned char nextOffset = 0;
    };

}