#include "cli.hpp"

int main (int argc, char** argv) {
    CodecServer::Cli* cli = new CodecServer::Cli();
    int rc = cli->main(argc, argv);
    delete cli;
    return rc;
}