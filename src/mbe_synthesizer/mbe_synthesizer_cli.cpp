#include "cli.hpp"

int main (int argc, char** argv) {
    CodecServer::Cli* cli = new CodecServer::Cli();
    return cli->main(argc, argv);
}