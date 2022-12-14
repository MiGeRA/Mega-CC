
#include "asm_mac.i"

| extern u16 CCRAM_Init(u32 fill);
func CCRAM_Init
    move.l  4(%sp), %d1
    lea     0x080000, %a0
_yet_wr:    
    movep.l %d1, 1(%a0)
    addq.l  #8, %a0
    cmpa.l  #0x090000, %a0
    bne     _yet_wr
    
    sub.l   %d0, %d0
    move.l  %d1, %a1
    lea     0x080000, %a0
_yet_rd:    
    movep.l 1(%a0), %d1
    addq.l  #8, %a0
    cmpa.l  %d1, %a1
    beq     _no_err
    addq    #1, %d0
_no_err:
    cmpa.l   #0x090000, %a0
    bne     _yet_rd
    rts

| extern u16 CCRAM_u16_rd(u32 offset);
func CCRAM_u16_rd
    move.l  4(%sp),%d1
    add.l   %d1,%d1
    lea     0x080001,%a0
    lea     (%a0,%d1.l),%a0
    moveq   #0,%d0
    movep.w 0(%a0),%d0              | return over reg.d0
    rts

| extern u32 CCRAM_u32_rd(u32 offset);
func CCRAM_u32_rd
    move.l  4(%sp),%d1
    add.l   %d1,%d1
    lea     0x080001,%a0
    lea     (%a0,%d1.l),%a0
    movep.l 0(%a0),%d0              | return over reg.d0
    rts

| extern void CCRAM_u16_wr(u32 offset, u16 val);
func CCRAM_u16_wr
    move.l  4(%sp),%d1
    add.l   %d1,%d1
    move.l  8(%sp),%d0              | values on stack are always long
    lea     0x080001,%a0
    lea     (%a0,%d1.l),%a0
    movep.w %d0,0(%a0)
    rts

| extern void CCRAM_u32_wr(u32 offset, u32 val);
func CCRAM_u32_wr
    move.l  4(%sp),%d1
    add.l   %d1,%d1
    move.l  8(%sp),%d0              | values on stack are always long
    lea     0x080001,%a0
    lea     (%a0,%d1.l),%a0
    movep.l %d0,0(%a0)
    rts
