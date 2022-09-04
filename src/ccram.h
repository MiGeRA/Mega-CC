#ifndef _CCRAM_H_
#define _CCRAM_H_

#define CCRAM_BASE 0x080001

u8 CCRAM_u8_rd(u32 offset);
u16 CCRAM_u16_rd(u32 offset);
u32 CCRAM_u32_rd(u32 offset);
void CCRAM_u8_wr(u32 offset, u8 val);
void CCRAM_u16_wr(u32 offset, u16 val);
void CCRAM_u32_wr(u32 offset, u32 val);
u16 CCRAM_Init(u32 fill);

#endif // _CCRAM_H_
