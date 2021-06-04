#include "ringbuffer.hpp"
#include <cstdlib>
#include <cstring>

using namespace Digiham;

Ringbuffer::Ringbuffer(size_t size) {
    this->size = size;
    data = (char*) malloc(sizeof(char) * size);
}

Ringbuffer::~Ringbuffer() {
    delete data;
}

size_t Ringbuffer::available(size_t read_pos) {
    return mod(write_pos - read_pos, size);
}

// modulo that will respect the sign
size_t Ringbuffer::mod(size_t n, size_t x) {
    return ((n%x)+x)%x;
}

size_t Ringbuffer::writeable() {
    if (write_pos >= size) {
        return size;
    }
    return size - write_pos;
}

char* Ringbuffer::getWritePointer() {
    return data + write_pos;
}

void Ringbuffer::advance(size_t& pos, size_t how_much) {
    pos = (pos + how_much) % size;
}

void Ringbuffer::advance(size_t how_much) {
    advance(write_pos, how_much);
}

void Ringbuffer::read(char* dst, size_t read_pos, size_t how_much) {
    if (read_pos + how_much > size) {
        size_t split = size - read_pos;
        memcpy(dst, data + read_pos, split);
        memcpy(dst + split, data, how_much - split);
    } else {
        memcpy(dst, data + read_pos, how_much);
    }
}

size_t Ringbuffer::getSize() {
    return size;
}