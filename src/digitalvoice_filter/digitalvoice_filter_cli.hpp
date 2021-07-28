#include "cli.hpp"

namespace Digiham::DigitalVoice {

    class Cli: public Digiham::Cli<short, short> {
        protected:
            std::string getName() override;
            Csdr::Module<short, short>* buildModule() override;
    };

}