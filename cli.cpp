#include "cli.hpp"
#include <codecserver/handshake.pb.h>
#include <codecserver/request.pb.h>
#include <codecserver/response.pb.h>
#include <codecserver/data.pb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <google/protobuf/any.pb.h>
#include <getopt.h>

using namespace CodecServer;
using namespace CodecServer::proto;

int Cli::main(int argc, char** argv) {
    if (!parseOptions(argc, argv)) {
        return 0;
    }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char* socket_path = "/tmp/codecserver.sock";
    strncpy(addr.sun_path, socket_path, strlen(socket_path));

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "socket error\n";
        return 1;
    }

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        std::cerr << "connection failure\n";
        return 1;
    }

    connection = new Connection(sock);
    google::protobuf::Any* message = connection->receiveMessage();
    if (!message->Is<Handshake>()) {
        std::cerr << "unexpected message\n";
        return 1;
    }

    Handshake handshake;
    message->UnpackTo(&handshake);

    std::cerr << "received handshake from " << handshake.servername() << "\n";

    if (!connection->isCompatible(handshake.serverversion())) {
        std::cerr << "server version mismatch\n";
        return 1;
    }

    Request request;
    request.set_codec("ambe");
    Settings* settings = request.mutable_settings();
    settings->clear_directions();
    settings->mutable_directions()->Add(Settings_Direction_DECODE);
    (*settings->mutable_args())["index"] = "33";
    connection->sendMessage(&request);

    message = connection->receiveMessage();
    if (!message->Is<Response>()) {
        std::cerr << "response error\n";
        return 1;
    }

    Response response;
    message->UnpackTo(&response);

    if (response.result() != Response_Status_OK) {
        std::cerr << "server replied with error, message: " << response.message() << "\n";
        return 1;
    }

    if (!response.has_framing()) {
        std::cerr << "framing info is not available\n";
        return 1;
    }
    framing = response.framing();

    std::cerr << "server response OK, start decoding!\n";

    char* in_buf = (char*) malloc(framing.channelbytes());
    fd_set read_fds;
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    int rc;
    int nfds = std::max(fileno(stdin), sock) + 1;

    while (run) {
        FD_ZERO(&read_fds);
        FD_SET(fileno(stdin), &read_fds);
        FD_SET(sock, &read_fds);

        rc = select(nfds, &read_fds, NULL, NULL, &tv);
        if (rc == -1) {
            std::cerr << "select() error\n";
            run = false;
        } else if (rc) {
            if (FD_ISSET(fileno(stdin), &read_fds)) {
                if (yaesu) {
                    char new_mode;
                    fread(&new_mode, sizeof(char), 1, stdin);
                    if (new_mode != mode) {
                        switchMode(new_mode);
                    }
                }
                size_t size = fread(in_buf, sizeof(char), getFrameSize(), stdin);
                if (size <= 0) {
                    run = false;
                    break;
                }
                connection->sendChannelData(in_buf, size);
            }
            if (FD_ISSET(sock, &read_fds)) {
                google::protobuf::Any* message = connection->receiveMessage();
                if (message->Is<SpeechData>()) {
                    SpeechData* data = new SpeechData();
                    message->UnpackTo(data);
                    std::string output = data->data();
                    fwrite(output.data(), sizeof(char), output.length(), stdout);
                    fflush(stdout);
                    delete data;
                } else if (message->Is<Response>()) {
                    Response* response = new Response();
                    message->UnpackTo(response);
                    if (response->has_framing()) {
                        framing = response->framing();
                    }
                    delete response;
                } else {
                    std::cerr << "received unexpected message type\n";
                }
            }
        } else {
            // no data, just timeout.
        }

    }

    free(in_buf);
    delete connection;

    ::close(sock);

    return 0;
}

unsigned char Cli::getFrameSize() {
    if (yaesu) {
        switch (mode) {
            case 0:
                return 9;
            case 2:
                return 7;
            case 3:
                return 18;
        }
    }
    return framing.channelbytes();
}

void Cli::switchMode(unsigned char new_mode) {
    if (new_mode > 3) {
        std::cerr << "invalid mode: " << +new_mode << "\n";
        return;
    }
    Renegotiation reneg;
    Settings* settings = reneg.mutable_settings();
    settings->clear_directions();
    settings->mutable_directions()->Add(Settings_Direction_DECODE);
    google::protobuf::Map<std::string, std::string>* args = settings->mutable_args();
    switch (new_mode) {
        case 0:
            (*args)["index"] = "33";
            break;
        case 2:
            (*args)["index"] = "34";
            break;
        case 3:
            (*args)["ratep"] = "0558:086b:1030:0000:0000:0190";
            break;
    }

    connection->sendMessage(&reneg);
    mode = new_mode;
}

void Cli::printUsage() {
    std::cerr <<
        "mbe_synthesizer version " << VERSION << "\n\n" <<
        "Usage: mbe_synthesizer [options]\n\n" <<
        "Available options:\n" <<
        " -h, --help              show this message\n" <<
        " -v, --version           print version and exit\n" <<
        " -y, --yaesu             activate YSF mode (allows in-stream switching of different mbe codecs)\n";
}

void Cli::printVersion() {
    std::cout << "mbe_synthesizer version " << VERSION << "\n";
}

bool Cli::parseOptions(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"yaesu", no_argument, NULL, 'y'},
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "yvh", long_options, NULL)) != -1 ) {
        switch (c) {
            case 'y':
                std::cerr << "enabling codec switching support for yaesu\n";
                yaesu = true;
                break;
            case 'v':
                printVersion();
                return false;
            case 'h':
                printUsage();
                return false;
        }
    }

    return true;
}