#pragma once

#include <cstdint>
#include <csdr/writer.hpp>

#define MAX_MESSAGE_LENGTH 80

namespace Digiham::Pocsag {

    class Message {
        public:
            Message(uint32_t address, unsigned char type);
            ~Message();
            void serialize(Csdr::Writer<unsigned char>* writer);
            void append(uint32_t data);
        private:
            uint32_t address;
            unsigned char type;
            char* content;
            int pos = 0;
    };

}