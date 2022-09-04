#include "genesis.h"
#include "ccrom.h"

/*
u16 __attribute__((noinline, used, longcall, section(".data"))) CCROM_Info(u16 *addr) // In-RAM function
{
    volatile u16 buff;
    asm volatile("move %sr, %d1");            // SYS_disableInts(); ...
    asm volatile("move #0x2700, %sr");        // ... Ok!
    asm volatile("move.w #0x0100, 0xA11100"); // Z80_requestBus(TRUE); ...
    asm volatile("move.w #0x0100, 0xA11200");
    asm volatile("_ws: btst.b #0, 0xA11100"); // ... Ok!
    asm volatile("bne.s _ws");
    asm volatile("move #0xAA, 0x00AAAA"); // for other chips only ...
    asm volatile("move #0x55, 0x005554"); // for other chips only ...
    asm volatile("move #0x90, 0x00AAAA"); // for PA28Fxxx only this requare
    asm volatile("move (%1), %0"
                 : "=d"(buff)
                 : "a"(addr));
    asm volatile("move #0xFF, 0");           // back to read-mode
    asm volatile("move.w #0x000, 0xA11100"); // Z80_releaseBus();
    asm volatile("move %d1, %sr");           // SYS_enableInts();
    return (buff);
}
*/
