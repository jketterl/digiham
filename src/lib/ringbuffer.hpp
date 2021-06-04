#pragma once

#include <cstddef>

namespace Digiham {

    class Ringbuffer {
        public:
            Ringbuffer(size_t size);
            ~Ringbuffer();
            size_t available(size_t read_pos);
            void read(char* dst, size_t read_pos, size_t how_much);
            size_t writeable();
            char* getWritePointer();
            void advance(size_t how_much);
            void advance(size_t& pos, size_t how_much);
            size_t getSize();
        private:
            size_t size;
            size_t write_pos = 0;
            size_t mod(size_t n, size_t x);
            char* data;
    };

}