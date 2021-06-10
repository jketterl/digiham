#pragma once

// Radio Control Channel
#define NXDN_RF_CHANNEL_TYPE_RCCH 0b00
// Radio Traffic Channel
#define NXDN_RF_CHANNEL_TYPE_RTCH 0b01
// Radio Direct Channel
#define NXDN_RF_CHANNEL_TYPE_RDCH 0b10
// Radio Traffic Channel / Composite Control Channel
#define NXDN_RF_CHANNEL_TYPE_RTCH_C 0b11

#define NXDN_CAC_TYPE_OUTBOUND_CAC 0b00
#define NXDN_CAC_TYPE_INBOUND_LONG 0b01
#define NXDN_CAC_TYPE_INBOUND_SHORT 0b11

// SACCH non-superframe
#define NXDN_USC_TYPE_SACCH_NON_SF 0b00
#define NXDN_USC_TYPE_UDCH 0b01
// SACCH superframe
#define NXDN_USC_TYPE_SACCH_SF 0b10
// SACCH superframe / idle
#define NXDN_USC_TYPE_SACCH_SF_IDLE 0b11

#define NXDN_DIRECTION_OUTBOUND 0
#define NXDN_DIRECTION_INBOUND 1

namespace Digiham::Nxdn {

    // Link Information Channel
    class Lich {
        public:
            static Lich* parse(unsigned char* raw);

            Lich(unsigned char data);
            unsigned char getRFType();
            unsigned char getFunctionalType();
            unsigned char getOption();
            unsigned char getDirection();
        private:
            unsigned char data;
    };

}