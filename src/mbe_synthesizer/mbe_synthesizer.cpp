#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <codecserver/proto/handshake.pb.h>
#include <codecserver/proto/request.pb.h>
#include <codecserver/proto/response.pb.h>
#include <codecserver/proto/data.pb.h>
#include "mbe_synthesizer.hpp"

using namespace Digiham::Mbe;
using namespace CodecServer::proto;

MbeSynthesizer::MbeSynthesizer(const std::string& host, unsigned short port) {
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

    std::stringstream ss;
    ss << port;
    const char* service = ss.str().c_str();

    int s = getaddrinfo(host.c_str(), service, &hints, &result);
    if (s != 0) {
        throw ConnectionError("getaddrinfo() failed: " + std::string(gai_strerror(s)));
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

        ::close(sock);
    }

    if (rp == NULL) {
        throw ConnectionError("could not connect to to server");
    }

    init(sock);
}

MbeSynthesizer::MbeSynthesizer(std::string path) {
    // unix domain sockets connection
    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    const char* socket_path = "/tmp/codecserver.sock";
    strncpy(addr.sun_path, socket_path, strlen(socket_path));

    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        throw ConnectionError("socket error: " + std::string(strerror(errno)));
    }

    if (connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        throw ConnectionError("connection failure: " + std::string(strerror(errno)));
    }

    init(sock);
}

MbeSynthesizer::MbeSynthesizer(): MbeSynthesizer("/tmp/codecserver.sock") {}

void MbeSynthesizer::init(int sock) {
    connection = new CodecServer::Connection(sock);
    handshake();
    readerThread = new std::thread([this] () { readLoop(); });
}

MbeSynthesizer::~MbeSynthesizer() {
    run = false;
    if (connection != nullptr) {
        connection->close();
        delete connection;
    }
    if (readerThread != nullptr) {
        readerThread->join();
        delete readerThread;
    }
}

void MbeSynthesizer::handshake() {
    google::protobuf::Any* message = connection->receiveMessage();

    if (message == nullptr) {
        throw ConnectionError("no response");
    }

    if (!message->Is<Handshake>()) {
        throw ConnectionError("unexpected message");
    }

    Handshake handshake;
    message->UnpackTo(&handshake);
    delete message;

    std::cerr << "received handshake from " << handshake.servername() << "\n";

    if (!connection->isCompatible(handshake.serverversion())) {
        throw ConnectionError("server version mismatch");
    }

    Request request;
    request.set_codec("ambe");
    Settings* settings = request.mutable_settings();
    settings->clear_directions();
    settings->mutable_directions()->Add(Settings_Direction_DECODE);
    // TODO figure out how to control this
    //if (yaesu) {
    //    (*settings->mutable_args())["index"] = "34";
    //} else if (dstar) {
        (*settings->mutable_args())["ratep"] = "0130:0763:4000:0000:0000:0048";
    //} else {
    //    (*settings->mutable_args())["index"] = "33";
    //}
    connection->sendMessage(&request);

    message = connection->receiveMessage();

    if (message == nullptr) {
        throw ConnectionError("no response");
    }

    if (!message->Is<Response>()) {
        throw ConnectionError("response error");
    }

    Response response;
    message->UnpackTo(&response);
    delete message;

    if (response.result() != Response_Status_OK) {
        throw ConnectionError("server replied with error, message: " + response.message());
    }

    if (!response.has_framing()) {
        throw ConnectionError("framing info is not available");
    }

    framing = response.framing();
}

bool MbeSynthesizer::canProcess() {
    return reader->available() > framing.channelbytes();
}

void MbeSynthesizer::process() {
    connection->sendChannelData((char*) reader->getReadPointer(), framing.channelbytes());
}

void MbeSynthesizer::readLoop() {
    while (run) {
        google::protobuf::Any* message = connection->receiveMessage();
        if (message == nullptr) break;

        if (message->Is<SpeechData>()) {
            SpeechData* data = new SpeechData();
            message->UnpackTo(data);
            std::string output = data->data();
            if (writer->writeable() * sizeof(short) < output.length()) {
                std::cerr << "dropping speech data due to writer overflow";
            } else {
                std::memcpy(writer->getWritePointer(), output.data(), output.length());
                writer->advance(output.length() / 2);
            }
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
}