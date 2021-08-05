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

            static constexpr unsigned char tact_positions[7] = { 0, 4, 8, 12, 14, 18, 22 };
            static constexpr unsigned char payload_positions[17] = {
                    1, 2, 3, 5, 6, 7, 9, 10, 11, 13, 15, 16, 17, 19, 20, 21, 23
            };
    };

}