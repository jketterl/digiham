#pragma once

namespace Digiham::Dmr {

    class Tact {
        public:
            static Tact* parse(unsigned char data);
            bool isBusy();
            unsigned char getSlot();
            unsigned char getLcss();
        private:
            explicit Tact(unsigned char data);
            unsigned char data;
    };

}