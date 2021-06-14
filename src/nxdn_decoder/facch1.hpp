#pragma once

namespace Digiham::Nxdn {

    class Facch1 {
        public:
            static Facch1* parse(unsigned char* raw);

            Facch1(unsigned char* data);
            ~Facch1();
            unsigned char getMessageType();
        private:
            static void deinterleave(unsigned char* input, unsigned char* output);
            static void inflate(unsigned char* input, unsigned char* output);
            static bool check_crc(unsigned char* in);

            unsigned char* data;
    };

}