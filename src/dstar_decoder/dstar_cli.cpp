#include "dstar_cli.hpp"

#include "dstar_decoder.hpp"

using namespace Digiham::DStar;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Digiham::Decoder* Cli::buildModule() {
    auto decoder = new Decoder();
    decoder->setMetaFile(metaFile);
    return decoder;
}

std::string Cli::getName() {
    return "dstar_decoder";
}
