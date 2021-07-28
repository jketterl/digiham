#include "dc_block_cli.hpp"
#include "dc_block.hpp"

using namespace Digiham::DcBlock;

int main(int argc, char** argv) {
    Cli runner;
    return runner.main(argc, argv);
}

Csdr::Module<float, float> * Cli::buildModule() {
    return new DcBlock();
}

std::string Cli::getName() {
    return "dc_block";
}
