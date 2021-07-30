#pragma once

#include <string>

namespace Digiham {

    namespace Mbe {

        class Mode {
            public:
                virtual ~Mode() = default;
        };

        class TableMode: public Mode {
            public:
                explicit TableMode(unsigned int index);
                unsigned int getIndex() const;
            private:
                const unsigned int index;
        };

        class ControlWordMode: public Mode {
            public:
                explicit ControlWordMode(short* cwds);
                ~ControlWordMode() override;
                short* getCwds();
                std::string getCwdsAsString();
            private:
                short* cwds;
        };

    }

}