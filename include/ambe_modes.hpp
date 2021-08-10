#pragma once

#include <string>
#include <functional>

namespace Digiham {

    namespace Mbe {

        class Mode {
            public:
                virtual ~Mode() = default;
                virtual bool operator==(const Mode& other) = 0;
        };

        class TableMode: public Mode {
            public:
                explicit TableMode(unsigned int index);
                unsigned int getIndex() const;
                bool operator==(const Mode& other) override;
            private:
                const unsigned int index;
        };

        class ControlWordMode: public Mode {
            public:
                explicit ControlWordMode(short* cwds);
                ~ControlWordMode() override;
                short* getCwds() const;
                std::string getCwdsAsString();
                bool operator==(const Mode& other) override;
            private:
                short* cwds;
        };

        class DynamicMode: public Mode {
            public:
                explicit DynamicMode(std::function<Mode*(unsigned char code)> callback);
                Mode* getModeFor(unsigned char code);
                bool operator==(const Mode& other) override;
            private:
                std::function<Mode*(unsigned char code)> callback;
        };

    }

}