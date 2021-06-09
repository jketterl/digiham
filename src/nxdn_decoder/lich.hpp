#pragma once

namespace Digiham::Nxdn {

    // Link Information Channel
    class Lich {
        public:
            static Lich* parse(unsigned char* raw);

            Lich(unsigned char data);
        private:
            unsigned char data;
    };

}