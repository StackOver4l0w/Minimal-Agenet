#pragma once

#define CMD_OPEN_SHELL          0x01
#define CMD_WRITE_SHELL         0x02
#define CMD_READ_SHELL          0x03
#define CMD_CLOSE_SHELL         0x04

#define CMD_LIST_DIRECTORY      0x05
#define CMD_READ_FILE           0x06
#define CMD_HASH_FILE           0x07

#define CMD_GET_DISPLAYS        0x08
#define CMD_GET_SCREENSHOT      0x09
#define CMD_EXIT                0x0A

#define STATUS_OK               0
#define STATUS_ERROR            1

#define IDENTITY_FRAME_SIZE     754
#define ID_HOSTNAME_SIZE        256
#define ID_USERNAME_SIZE        256
#define ID_ARCH_SIZE            32
#define ID_PLATFORM_SIZE        32
#define ID_OS_VERSION_SIZE      128
#define ID_COMMIT_HASH_SIZE     9
#define ID_API_VERSION          5
#define ID_AGENT_NAME_ID        1

#ifndef ID_BUILD_NUMBER
#define ID_BUILD_NUMBER         1
#endif

#define SHELL_POOL_SIZE         256
#define SHELL_READ_CHUNK        65536

#define RECV_FRAGMENT_SIZE      65536

#define MAX_MESSAGE_SIZE        65536

#define HEXDUMP_LIMIT           64

#define RC_EXIT                 0

#define RC_SESSION_LOST         2

#define RC_LOCAL_ERROR          1
