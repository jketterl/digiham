#pragma once

#include <codecserver/connection.hpp>
#include <codecserver/proto/framing.pb.h>
#include <string>

using namespace CodecServer::proto;

namespace CodecServer {

    class Cli {
        public:
            int main(int argc, char** argv);
        private:
            void printUsage();
            void printVersion();
            bool parseOptions(int argc, char** argv);
            void switchMode(unsigned char mode);
            unsigned char getFrameSize();
            int buildSocket();
            Connection* connection;
            bool run = true;
            bool yaesu = false;
            bool dstar = false;
            unsigned char mode = 2;
            FramingHint framing;
            std::string server = "/tmp/codecserver.sock";
            bool testOnly = false;
    };

}