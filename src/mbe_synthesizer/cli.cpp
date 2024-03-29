#include "cli.hpp"
#include <codecserver/proto/handshake.pb.h>
#include <codecserver/proto/request.pb.h>
#include <codecserver/proto/response.pb.h>
#include <codecserver/proto/data.pb.h>
#include <codecserver/proto/check.pb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/select.h>
#include <unistd.h>
#include <netdb.h>
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

    int sock = buildSocket();
    if (sock == -1) {
        return 1;
    }

    connection = new Connection(sock);
    google::protobuf::Any* message = connection->receiveMessage();

    if (message == nullptr) {
        std::cerr << "no response\n";
        delete connection;
        return 1;
    }

    if (!message->Is<Handshake>()) {
        std::cerr << "unexpected message\n";
        delete connection;
        return 1;
    }

    Handshake handshake;
    message->UnpackTo(&handshake);
    delete message;

    std::cerr << "received handshake from " << handshake.servername() << "\n";

    if (!connection->isCompatible(handshake.protocolversion())) {
        std::cerr << "protocol version mismatch\n";
        delete connection;
        return 1;
    }

    if (testOnly) {
        Check check;
        check.set_codec("ambe");
        connection->sendMessage(&check);

        message = connection->receiveMessage();

        if (message == nullptr) {
            std::cerr << "no response\n";
            delete connection;
            return 1;
        }

        if (!message->Is<Response>()) {
            std::cerr << "unexpected response\n";
            delete message;
            delete connection;
            return 1;
        }

        Response response;
        message->UnpackTo(&response);
        delete message;

        if (response.result() != Response_Status_OK) {
            std::cerr << "server replied with error, message: " << response.message() << "\n";
            delete connection;
            return 1;
        }

        std::cerr << "server response ok\n";
        delete connection;
        return 0;
    }

    Request request;
    request.set_codec("ambe");
    Settings* settings = request.mutable_settings();
    settings->clear_directions();
    settings->mutable_directions()->Add(Settings_Direction_DECODE);
    if (yaesu) {
        (*settings->mutable_args())["index"] = "34";
    } else if (dstar) {
        (*settings->mutable_args())["ratep"] = "0130:0763:4000:0000:0000:0048";
    } else {
        (*settings->mutable_args())["index"] = "33";
    }
    connection->sendMessage(&request);

    message = connection->receiveMessage();

    if (message == nullptr) {
        std::cerr << "no response\n";
        return 1;
    }

    if (!message->Is<Response>()) {
        std::cerr << "response error\n";
        return 1;
    }

    Response response;
    message->UnpackTo(&response);
    delete message;

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

    char* in_buf = (char*) malloc(1024);
    fd_set read_fds;
    struct timeval tv;
    int rc;
    int nfds = std::max(fileno(stdin), sock) + 1;

    while (run) {
        FD_ZERO(&read_fds);
        FD_SET(fileno(stdin), &read_fds);
        FD_SET(sock, &read_fds);
        tv.tv_sec = 10;
        tv.tv_usec = 0;

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
                if (message == nullptr) {
                    std::cerr << "no response\n";
                    run = false;
                } else if (message->Is<SpeechData>()) {
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
        //} else {
            // no data, just timeout.
        }

    }

    free(in_buf);
    delete connection;

    ::close(sock);

    return 0;
}

int Cli::buildSocket() {
    if (server.at(0) == '/') {
        // unix domain sockets connection
        sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        const char* socket_path = "/tmp/codecserver.sock";
        strncpy(addr.sun_path, socket_path, strlen(socket_path));

        int sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock == -1) {
            std::cerr << "socket error: " << strerror(errno) << "\n";
            return -1;
        }

        if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
            std::cerr << "connection failure: " << strerror(errno) << "\n";
            return -1;
        }

        return sock;
    } else {
        // IPv6 / IPv4 socket
        struct addrinfo hints;
        struct addrinfo* result;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* Stream socket */
        hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
        hints.ai_protocol = 0;           /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        size_t pos = server.find(":");
        char* service = (char *) "1073";
        if (pos != std::string::npos) {
            std::string serviceStr = server.substr(pos + 1);
            service = (char*) serviceStr.c_str();
            server = server.substr(0, pos);
        }

        int s = getaddrinfo(server.c_str(), service, &hints, &result);
        if (s != 0) {
            std::cerr << "getaddrinfo() failed: " << gai_strerror(s) << "\n";
            return -1;
        }

        struct addrinfo* rp;
        int sock;
        for (rp = result; rp != NULL; rp = rp->ai_next) {
            sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sock == -1) {
                continue;
            }

            if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
                break;                  /* Success */
            }

            close(sock);
        }

        if (rp == NULL) {
            std::cerr << "could not connect to to server\n";
            return -1;
        }

        freeaddrinfo(result);

        return sock;
    }
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
        " -y, --yaesu             activate YSF mode (allows in-stream switching of different mbe codecs)\n" <<
        " -d, --dstar             activate D-Star compatible codec\n" <<
        " -s, --server            codecserver to connect to (default: \"" << server << "\")\n" <<
        " -t, --test              test if codecserver can supply AMBE codec\n";
}

void Cli::printVersion() {
    std::cout << "mbe_synthesizer version " << VERSION << "\n";
}

bool Cli::parseOptions(int argc, char** argv) {
    int c;
    static struct option long_options[] = {
        {"yaesu", no_argument, NULL, 'y'},
        {"dstar", no_argument, NULL, 'd'},
        {"version", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {"server", required_argument, NULL, 's'},
        {"test", no_argument, NULL, 't'},
        { NULL, 0, NULL, 0 }
    };
    while ((c = getopt_long(argc, argv, "yvhs:dt", long_options, NULL)) != -1 ) {
        switch (c) {
            case 'y':
                std::cerr << "enabling codec switching support for yaesu\n";
                yaesu = true;
                break;
            case 'd':
                std::cerr << "enabling d-star codec\n";
                dstar = true;
                break;
            case 'v':
                printVersion();
                return false;
            case 'h':
                printUsage();
                return false;
            case 's':
                server = std::string(optarg);
                break;
            case 't':
                testOnly = true;
                break;
        }
    }

    return true;
}