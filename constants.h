#ifndef CONSTANTS_H__
#define CONSTANTS_H__

/* Application constants */
#define NAME "bfc4ntk"
#define VERSION "3.3"
#define BUILD_DATE __DATE__
#define AUTHOR "Tobi@s"
#define RELEASED_FOR "  http://dc.p-mc.eu/\n  http://www.goprawn.com/\n  http://dashcamtalk.com/"

/* Important offsets */
#define MAGIC_OFFSET 0x00
#define ALGO_OFFSET 0x04
#define CSIZE_OFFSET 0x0c
#define FILESIZE_OFFSET 0x68
#define BINCTRL_OFFSET 0x78 //1: PartComp 0: FullComp/raw
#define PC_OFFS_OFFSET 0x30c

/* Important constants*/
#define HEADER_LEN 0x10
#define BCL_MAGIC "BCL1"
#define LZ_ALGO 9
#define DWORD_SIZE 4

#endif
