#include "genesis.h"
#include "hexout.h"

void printhex8(u8 buff, u8 dig, u8 x, u8 y)
{
    char outbuff[dig + 2];

    intToHex(buff, outbuff, dig);
    VDP_drawTextBG(BG_A, outbuff, x, y);
}

void printhex8z(u8 buff, u8 dig, u8 x, u8 y)
{
    char outbuff[dig + 4];

    intToHex(buff, outbuff, dig);
    VDP_drawTextBG(BG_A, "0x", x, y);
    VDP_drawTextBG(BG_A, outbuff, x + 2, y);
}

void printhex16(u16 buff, u8 dig, u8 x, u8 y)
{
    char outbuff[dig + 2];

    intToHex(buff, outbuff, dig);
    VDP_drawTextBG(BG_A, outbuff, x, y);
}

void printhex16z(u16 buff, u8 dig, u8 x, u8 y)
{
    char outbuff[dig + 4];

    intToHex(buff, outbuff, dig);
    VDP_drawTextBG(BG_A, "0x", x, y);
    VDP_drawTextBG(BG_A, outbuff, x + 2, y);
}

void printhex32(u32 buff, u8 dig, u8 x, u8 y)
{
    char outbuff[dig + 2];

    intToHex(buff, outbuff, dig);
    VDP_drawTextBG(BG_A, outbuff, x, y);
}

void printhex32z(u32 buff, u8 dig, u8 x, u8 y)
{
    char outbuff[dig + 4];

    intToHex(buff, outbuff, dig);
    VDP_drawTextBG(BG_A, "0x", x, y);
    VDP_drawTextBG(BG_A, outbuff, x + 2, y);
}
