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

        class ConnectionError: public std::runtime_error {
            public:
                explicit ConnectionError(const std::string& reason): std::runtime_error(reason) {}
        };

        class MbeSynthesizer: public Csdr::Module<unsigned char, short> {
            public:
                MbeSynthesizer(Mode* mode);
                explicit MbeSynthesizer(std::string path, Mode* mode);
                MbeSynthesizer(const std::string& host, unsigned short port, Mode* mode);
                ~MbeSynthesizer() override;
                bool canProcess() override;
                void process() override;
            private:
                void init();
                void handshake();
                void readLoop();
                void setDynamicMode(Mode* mode);
                void waitForResponse();
                Mode* mode;
                bool dynamicMode = false;
                Mode* currentMode = nullptr;
                int sock;
                CodecServer::Connection* connection = nullptr;
                CodecServer::proto::FramingHint framing;
                std::thread* readerThread = nullptr;
                bool run = true;
                std::mutex receiveMutex;
                std::condition_variable framingCV;
                std::mutex framingMutex;
        };

    }

}