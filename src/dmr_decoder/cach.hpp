#pragma once

#include "tact.hpp"

namespace Digiham::Dmr {

    class Cach {
        public:
            static Cach* parse(const unsigned char* raw);
            ~Cach();

            bool hasTact();
            Tact* getTact();
        private:
            explicit Cach(Tact* tact, unsigned char* payload);
            Tact* tact;
            unsigned char* payload;

            static const unsigned char tact_positions[7];
            static const unsigned char payload_positions[17];
    };

}