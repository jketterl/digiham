#pragma once

namespace Digiham::Nxdn {

    class Sacch {
        public:
            static Sacch* parse(unsigned char* data);
    };

}