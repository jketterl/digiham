#pragma once

#include <csdr/module.hpp>
#include <codecserver/connection.hpp>
#include <codecserver/proto/framing.pb.h>
#include <thread>

namespace Digiham {

    namespace Mbe {

        class ConnectionError: public std::runtime_error {
            public:
                explicit ConnectionError(const std::string& reason): std::runtime_error(reason) {}
        };

        class MbeSynthesizer: public Csdr::Module<unsigned char, short> {
            public:
                MbeSynthesizer();
                explicit MbeSynthesizer(std::string path);
                MbeSynthesizer(const std::string& host, unsigned short port);
                ~MbeSynthesizer() override;
                bool canProcess() override;
                void process() override;
            private:
                void init(int sock);
                void handshake();
                void readLoop();
                CodecServer::Connection* connection = nullptr;
                CodecServer::proto::FramingHint framing;
                std::thread* readerThread = nullptr;
                bool run = true;
        };

    }

}