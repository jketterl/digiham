#include "pocsag_cli.hpp"
#include "pocsag_decoder.hpp"

using namespace Digiham::Pocsag;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Digiham::Decoder* Cli::buildModule() {
    auto decoder = new Decoder();
    decoder->setMetaWriter(metaWriter);
    return decoder;
}

std::string Cli::getName() {
    return "pocsag_decoder";
}
