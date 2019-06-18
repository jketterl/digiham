#include <stdint.h>

// Wires-X command
#define COMMAND_DX_REQ        0x5d715f
#define COMMAND_CONN_REQ      0x5d235f
#define COMMAND_DISC_REQ      0x5d2a5f
#define COMMAND_ALL_REQ       0x5d665f
#define COMMAND_DX_RESP       0x5d515f
#define COMMAND_DX_RESP2      0x5d525f
#define COMMAND_CONN_RESP     0x5d415f
#define COMMAND_DISC_RESP     0x5d415f
#define COMMAND_ALL_RESP      0x5d465f

// GPS command
#define COMMAND_NULL0_GPS     0x22615f
#define COMMAND_SHORT_GPS     0x22625f
#define COMMAND_NULL1_GPS     0x47635f
#define COMMAND_LONG_GPS      0x47645f
