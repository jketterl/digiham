#pragma once

#define FRAME_TYPE_HEADER_CHANNEL 0
#define FRAME_TYPE_COMMUNICATION_CHANNEL 1
#define FRAME_TYPE_TERMINATOR_CHANNEL 2
#define FRAME_TYPE_TEST_CHANNEL 3

#define DATA_TYPE_VD_TYPE_1 0
#define DATA_TYPE_DATA_FR 1
#define DATA_TYPE_VD_TYPE_2 2
#define DATA_TYPE_VOICE_FR 3

namespace Digiham::Ysf {

    class Fich {
        public:
            static Fich* parse(unsigned char* data);
            unsigned char getFrameType() const;
            unsigned char getDataType() const;
            unsigned char getFrameNumber() const;
        private:
            explicit Fich(uint32_t data);
            uint32_t data;
    };

}