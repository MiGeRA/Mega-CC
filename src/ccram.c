#include "genesis.h"
#include "ccram.h"

u8 CCRAM_u8_rd(u32 offset)
{
    return *(vu8*)(CCRAM_BASE + (offset * 2));
}

void CCRAM_u8_wr(u32 offset, u8 val)
{
    *(vu8*)(CCRAM_BASE + (offset * 2)) = val;
}

u8 CCRAM_init()
{
    volatile u8 err = 0;
    for (u16 i = 0; i < 32768; i++)
        CCRAM_u8_wr(i, 0x5A);
    for (u16 i = 0; i < 32768; i++)
        if (CCRAM_u8_rd(i) != 0x5A)
            err++;

    if (err)
        return (err);

    for (u16 i = 0; i < 32768; i++)
        CCRAM_u8_wr(i, 0);
    for (u16 i = 0; i < 32768; i++)
        if (CCRAM_u8_rd(i) != 0)
            err++;

    return (err);
}
