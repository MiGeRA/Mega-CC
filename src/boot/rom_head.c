#include "genesis.h"
#define ENABLE_BANK_SWITCH 0

__attribute__((externally_visible))
const ROMHeader rom_header = {
#if (ENABLE_BANK_SWITCH != 0)
    "SEGA SSF        ",
#else
    "SEGA MEGA DRIVE ",
#endif
    "(C)SGDK 2022    ",
    "MEGA Code Cracker (Open Source Project)         ",
    "MEGA-CC OSP                                     ",
    "GM DEADC0DE-00",
    0x000,
    "J               ",
    0x00000000,
#if (ENABLE_BANK_SWITCH != 0)
    0x003FFFFF,
#else
    0x0003FFFF,
#endif
    0xE0FF0000,
    0xE0FFFFFF,
    "RA",
    0xB820,
    0x00080000,
    0x0008FFFF,
    "            ",
    "https://github.com/MiGeRA/Mega-CC       ",
    "JUE             "
};