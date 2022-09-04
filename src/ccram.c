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

/*
u16 CCRAM_Init(u8 fill)
{
    volatile u16 err = 0;
    for (u16 i = 0; i < 32768; i++)
        CCRAM_u8_wr(i, fill);
    for (u16 i = 0; i < 32768; i++)
        if (CCRAM_u8_rd(i) != fill)
            err++;
    return (err);
}
*/
