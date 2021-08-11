#pragma once

#include <cstdint>

#define CODEWORD_SIZE 32
#define CODEWORDS_PER_SYNC 16

namespace Digiham::Pocsag {

    class Codeword {
        public:
            static Codeword* parse(unsigned char* input);

            bool isIdle() const;
            uint32_t getPayload() const;
            bool isAddressCodeword() const;
            uint32_t getAddress() const;
            unsigned char getFunctionBits() const;
        private:
            explicit Codeword(uint32_t data);
            uint32_t data;
            const uint32_t idle_codeword = 0b01111010100010011100000110010111;
    };

}