#include "ysf_cli.hpp"

using namespace Digiham::Ysf;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

std::string Cli::getName() {
    return "ysf_decoder";
}

Decoder *Cli::buildModule() {
    auto module = new Decoder();
    module->setMetaWriter(metaWriter);
    return module;
}