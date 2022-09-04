
#include "asm_dat.i"

| extern u16 CCROM_Info(u16 *addr);
func CCROM_Info
    move.l  4(%sp), %a0
    move    %sr, %d1
    move    #0x2700, %sr
    move.w  #0x0100, 0xA11100
    move.w  #0x0100, 0xA11200
_ws:
    btst.b  #0, 0xA11100
    bne.s   _ws
    move    #0xAA, 0x00AAAA
    move    #0x55, 0x005554
    move    #0x90, 0x00AAAA
    move    (%a0), %d0
    move    #0xFF, 0
    move.w  #0x000, 0xA11100
    move    %d1, %sr
    rts
