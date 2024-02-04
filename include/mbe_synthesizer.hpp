#pragma once

#include <csdr/module.hpp>
#include <codecserver/connection.hpp>
#include <codecserver/proto/framing.pb.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ambe_modes.hpp"

namespace Digiham {

    namespace Mbe {

        class Error: public std::runtime_error {
            public:
                explicit Error(const std::string& reason): std::runtime_error(reason) {}
        };

        class ConnectionError: public Error {
            public:
                explicit ConnectionError(const std::string& reason): Error(reason) {}
        };

        class ProtocolError: public Error {
            public:
                explicit ProtocolError(const std::string& reason): Error(reason) {}
        };

        class VersionError: public Error {
            public:
                explicit VersionError(const std::string& reason): Error(reason) {}
        };

        class ServerError: public Error {
            public:
                explicit ServerError(const std::string& reason): Error(reason) {}
        };

        class FramingError: public Error {
            public:
                explicit FramingError(const std::string& reason): Error(reason) {}
        };

        class MbeSynthesizer: public Csdr::Module<unsigned char, short> {
            public:
                explicit MbeSynthesizer();
                explicit MbeSynthesizer(const std::string& path);
                MbeSynthesizer(const std::string& host, unsigned short port);
                ~MbeSynthesizer() override;
                void setMode(Mode* mode);
                bool hasAmbeCodec();
                bool canProcess() override;
                void process() override;
            private:
                explicit MbeSynthesizer(int sock);
                static int connect(const std::string& path);
                static int connect(const std::string& host, unsigned short port);
                void handshake();
                void request();
                void readLoop();
                void setDynamicMode(Mode* mode);
                Mode* mode = nullptr;
                bool dynamicMode = false;
                Mode* currentMode = nullptr;
                int sock;
                CodecServer::Connection* connection = nullptr;
                CodecServer::proto::FramingHint framing;
                std::thread* readerThread = nullptr;
                bool run = true;
                std::condition_variable framingCV;
                std::mutex framingMutex;
        };

    }

}