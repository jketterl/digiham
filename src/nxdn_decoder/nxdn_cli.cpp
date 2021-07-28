#include "nxdn_cli.hpp"
#include "nxdn_decoder.hpp"

using namespace Digiham::Nxdn;

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
    return "nxdn_decoder";
}
