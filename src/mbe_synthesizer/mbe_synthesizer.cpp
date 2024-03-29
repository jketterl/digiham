#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <chrono>
#include <codecserver/proto/handshake.pb.h>
#include <codecserver/proto/request.pb.h>
#include <codecserver/proto/response.pb.h>
#include <codecserver/proto/data.pb.h>
#include <codecserver/proto/check.pb.h>
#include "mbe_synthesizer.hpp"

using namespace Digiham::Mbe;
using namespace CodecServer::proto;

int MbeSynthesizer::connect(const std::string &host, unsigned short port) {
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

        struct timeval timeout {
            .tv_sec = 5,
            .tv_usec = 0
        };
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        if (::connect(sock, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;                  /* Success */
        }

        ::close(sock);
    }

    freeaddrinfo(result);

    if (rp == NULL) {
        throw ConnectionError("could not connect to to server");
    }

    return sock;
}

MbeSynthesizer::MbeSynthesizer(const std::string& host, unsigned short port):
    MbeSynthesizer(MbeSynthesizer::connect(host, port))
{}

int MbeSynthesizer::connect(const std::string &path) {
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

    struct timeval timeout {
        .tv_sec = 5,
        .tv_usec = 0
    };
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (::connect(sock, (struct sockaddr*) &addr, sizeof(addr)) == -1) {
        throw ConnectionError("connection failure: " + std::string(strerror(errno)));
    }

    return sock;
}

MbeSynthesizer::MbeSynthesizer(const std::string& path):
    MbeSynthesizer(MbeSynthesizer::connect(path))
{}

MbeSynthesizer::MbeSynthesizer(): MbeSynthesizer("/tmp/codecserver.sock") {}

MbeSynthesizer::MbeSynthesizer(int sock):
    sock(sock),
    connection(new CodecServer::Connection(sock))
{
    handshake();
}

void MbeSynthesizer::setMode(Mode* mode) {
    this->mode = mode;
    dynamicMode = dynamic_cast<DynamicMode*>(mode) != nullptr;
    request();
    readerThread = new std::thread([this] () { readLoop(); });
}

MbeSynthesizer::~MbeSynthesizer() {
    run = false;
    if (connection != nullptr) {
        connection->close();
    }
    if (readerThread != nullptr) {
        readerThread->join();
        delete readerThread;
        readerThread = nullptr;
    }
    delete connection;
    connection = nullptr;
    // avoid double delete by checking for identity first
    if (currentMode != mode) {
        delete currentMode;
    }
    delete mode;
    currentMode = nullptr;
    mode = nullptr;
}

void MbeSynthesizer::handshake() {
    google::protobuf::Any* message = connection->receiveMessage();

    if (message == nullptr) {
        throw ProtocolError("no handshake");
    }

    if (!message->Is<Handshake>()) {
        throw ProtocolError("unexpected message");
    }

    Handshake handshake;
    message->UnpackTo(&handshake);
    delete message;

    if (!connection->isCompatible(handshake.protocolversion())) {
        throw VersionError("server protocol version is incompatible");
    }
}

bool MbeSynthesizer::hasAmbeCodec() {
    Check check;
    check.set_codec("ambe");
    connection->sendMessage(&check);

    google::protobuf::Any* message = connection->receiveMessage();

    if (message == nullptr) {
        delete message;
        throw ProtocolError("no response to codec check");
    }

    if (!message->Is<Response>()) {
        delete message;
        throw ProtocolError("response error");
    }

    Response response;
    message->UnpackTo(&response);
    delete message;

    return response.result() == Response_Status_OK;
}

void MbeSynthesizer::request() {
    Request request;
    request.set_codec("ambe");
    Settings* settings = request.mutable_settings();
    settings->clear_directions();
    settings->mutable_directions()->Add(Settings_Direction_DECODE);
    TableMode* tmode;
    ControlWordMode* cmode;
    DynamicMode* dmode;

    currentMode = mode;
    if ((dmode = dynamic_cast<DynamicMode*>(mode))) {
        currentMode = dmode->getModeFor(0);
    }

    if ((tmode = dynamic_cast<TableMode*>(currentMode))) {
        (*settings->mutable_args())["index"] = std::to_string(tmode->getIndex());
    } else if ((cmode = dynamic_cast<ControlWordMode*>(currentMode))) {
        (*settings->mutable_args())["ratep"] = cmode->getCwdsAsString();
    }
    connection->sendMessage(&request);

    google::protobuf::Any* message = connection->receiveMessage();

    if (message == nullptr) {
        throw ProtocolError("no response to codec request");
    }

    if (!message->Is<Response>()) {
        throw ProtocolError("response error");
    }

    Response response;
    message->UnpackTo(&response);
    delete message;

    if (response.result() != Response_Status_OK) {
        throw ServerError(response.message());
    }

    if (!response.has_framing()) {
        throw FramingError("framing info is not available");
    }

    framing = response.framing();
}

bool MbeSynthesizer::canProcess() {
    std::lock_guard<std::mutex> lock(processMutex);
    return reader->available() > framing.channelbytes();
}

void MbeSynthesizer::process() {
    std::lock_guard<std::mutex> lock(processMutex);
    if (dynamicMode) {
        auto code = *(reader->getReadPointer());
        reader->advance(1);
        auto newMode = ((DynamicMode*) mode)->getModeFor(code);
        if (newMode != nullptr) {
            setDynamicMode(newMode);
        }
    }
    unsigned int bytes = framing.channelbytes();
    connection->sendChannelData((char*) reader->getReadPointer(), bytes);
    reader->advance(bytes);
}

void MbeSynthesizer::readLoop() {
    while (run) {
        google::protobuf::Any* message = nullptr;
        if (connection != nullptr) {
            message = connection->receiveMessage();
        }
        if (message == nullptr) break;

        if (message->Is<SpeechData>()) {
            SpeechData *data = new SpeechData();
            message->UnpackTo(data);
            std::string output = data->data();
            if (writer->writeable() * sizeof(short) < output.length()) {
                std::cerr << "dropping speech data due to writer overflow\n";
            } else {
                std::memcpy(writer->getWritePointer(), output.data(), output.length());
                writer->advance(output.length() / 2);
            }
            delete data;
        } else if (message->Is<Response>()) {
            Response *response = new Response();
            message->UnpackTo(response);
            if (response->has_framing()) {
                framing = response->framing();
            }
            delete response;

            std::unique_lock<std::mutex> lk(framingMutex);
            framingCV.notify_all();
        } else {
            std::cerr << "received unexpected message type\n";
        }

        delete message;
    }
}

void MbeSynthesizer::setDynamicMode(Mode *mode) {
    if (currentMode == mode) {
        return;
    }

    if (*currentMode == *mode) {
        delete mode;
        return;
    }

    Renegotiation reneg;
    Settings* settings = reneg.mutable_settings();
    settings->clear_directions();
    settings->mutable_directions()->Add(Settings_Direction_DECODE);
    google::protobuf::Map<std::string, std::string>* args = settings->mutable_args();
    TableMode* tmode;
    ControlWordMode* cmode;
    if ((tmode = dynamic_cast<TableMode*>(mode))) {
        (*settings->mutable_args())["index"] = std::to_string(tmode->getIndex());
    } else if ((cmode = dynamic_cast<ControlWordMode*>(mode))) {
        (*settings->mutable_args())["ratep"] = cmode->getCwdsAsString();
    }

    // get the lock before sending the request
    // this way we avoid parsing the response before we reach the wait()
    std::unique_lock<std::mutex> lk(framingMutex);

    connection->sendMessage(&reneg);

    if (framingCV.wait_for(lk, std::chrono::seconds(10)) == std::cv_status::timeout) {
        throw FramingError("timeout waiting for framing information");
    }

    auto old = currentMode;
    currentMode = mode;
    delete old;
}
