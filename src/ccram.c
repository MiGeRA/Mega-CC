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
