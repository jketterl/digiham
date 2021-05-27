#pragma once

#include <codecserver/connection.hpp>
#include <codecserver/proto/framing.pb.h>

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
            Connection* connection;
            bool run = true;
            bool yaesu = false;
            unsigned char mode = 255;
            FramingHint framing;
    };

}