#include <genesis.h>
#include "hexout.h"
#include "font.h"

static void Print_Code_Table(u8 x, u8 y);
static void Print_Cursor_Position();
static void Reprint_Cursor();
static void ever_VInt();
static u8 CC_RAM_Init();

//#define RAMFUNC __attribute__((longcall, section(".data")))
//#define RAMFUNC __attribute__((longcall, section(".ramfunc")))

//__attribute__((section(".data")))
// void subroutine_name (void) __attribute__((section(".ramfunc")));

/*
RAMFUNC void testfunc_name(void)
{
}
*/

// Params section
#define maxcode 10 // max quantity codes
#define posx 12    // offset on screen for print codetable
#define posy 10    // offset on screen for print codetable

// Declarations section

u16 tbladdr[maxcode]; // array of address part of "code"
u8 tbldata[maxcode];  // array of data part of "code"
u8 tblstat[maxcode];  // array of status part of "code" and primus byte of address

u8 xm = 0; // x-position of menu cursor
u8 ym = 0; // y-position of menu cursor

u32 tick = 0; // growing counter
u8 mask = 0;  // time period flag

// u16 *address;           // for debug
// u16 datbuff;            // for debug

void Joy_Handler(u16 joy, u16 changed, u16 state)
{
    if (joy == JOY_1)
    {
        if (state & BUTTON_UP)
        {
            ym = ym == 0 ? maxcode - 1 : ym - 1;
            Print_Code_Table(posx, posy);
        }
        else if (state & BUTTON_DOWN)
        {
            ym = ym < maxcode - 1 ? ym + 1 : 0;
            Print_Code_Table(posx, posy);
        }
        else if (state & BUTTON_LEFT)
        {
            xm = xm == 0 ? 5 : xm - 1;
            Print_Code_Table(posx, posy);
        }
        else if (state & BUTTON_RIGHT)
        {
            xm = xm < 5 ? xm + 1 : 0;
            Print_Code_Table(posx, posy);
        }

        if (state & BUTTON_A)
        {
            if (xm < 4)
                tbladdr[ym] = tbladdr[ym] + (0x1000 >> (xm * 4));
            else
                tbldata[ym] = tbldata[ym] + (0x10 >> ((xm - 4) * 4));
            tblstat[ym] = 0xFF;
            Print_Code_Table(posx, posy);
        }
        if (state & BUTTON_B)
        {
            if (xm < 4)
                tbladdr[ym] = tbladdr[ym] - (0x1000 >> (xm * 4));
            else
                tbldata[ym] = tbldata[ym] - (0x10 >> ((xm - 4) * 4));
            tblstat[ym] = 0xFF;
            Print_Code_Table(posx, posy);
        }
        if (state & BUTTON_C)
        {
            tblstat[ym] = tblstat[ym] ? 0 : 0xFF;
            if (tblstat[ym] == 0)
                VDP_drawText(" ", posx + 11, posy + ym);
            else
                VDP_drawText("*", posx + 11, posy + ym);
        }

        if (state & BUTTON_START)
        {
            // Direct write - for example
            /*
            *(u8 *)(0x080001) = 0x01;

            *(u8 *)(0x080003) = 0xFF;
            *(u8 *)(0x080005) = 0x9C;
            *(u8 *)(0x080007) = 0x11;
            *(u8 *)(0x080009) = 0x7F;

            *(u8 *)(0x08000B) = 0x00;
            */

            ccramwr(0, 0); // start byte - null codes quantity
            for (u8 i = 0; i < maxcode; i++)
                if (tblstat[i])
                    ccramwr(0, 1); // start byte - not null codes quantity

            u8 ramcnt = 1; // init value CC-RAM counter

            // Fill CC-RAM
            for (u8 i = 0; i < maxcode; i++)
            {
                if (tblstat[i])
                {
                    ccramwr(ramcnt, tblstat[i]); // primus adrress byte - 0xFF if code actived
                    ramcnt++;
                    ccramwr(ramcnt, (u8)((tbladdr[i] & 0xFF00) >> 8)); // hi address byte
                    ramcnt++;
                    ccramwr(ramcnt, (u8)(tbladdr[i] & 0x00FF)); // low address byte
                    ramcnt++;
                    ccramwr(ramcnt, tbldata[i]); // data byte
                    ramcnt++;
                }
            }

            // Setting runned in-slot Cart flag
            *(u8 *)(0x088FFF) = 0xA5;

            // Switch mapper to in-slot Cart
            *(u8 *)(0x3FFFFF) = 0xA5;

            // Switch mapper to in-slot cart - alt variant
            /*
            asm volatile("LEA   	0x00FF0000, %A5");
            asm volatile("MOVEA.l	%A5, %A0");
            asm volatile("MOVE.w	#0x2639, (%A5)+");
            asm volatile("MOVE.l	#0x0003F0000, (%A5)+");
            asm volatile("MOVE.w	#0x4EF9, (%A5)+");
            asm volatile("MOVE.l	#0x00FF0000, (%A5)+");
            asm volatile("JMP	    (%A0)");
            */
        }
    }
}

int main(bool hardReset)
{
    /*
    for (u8 i = 0; i < sizeof(tbladdr); i++) tbladdr[i] = 0;
    for (u8 i = 0; i < sizeof(tbldata); i++) tbldata[i] = 0;
    for (u8 i = 0; i < sizeof(tblstat); i++) tblstat[i] = 0;
    */

    JOY_init();
    JOY_setEventHandler(&Joy_Handler);
    SYS_setVIntCallback(ever_VInt);

    VDP_setScreenWidth320();
    VDP_setHInterrupt(0);
    VDP_setHilightShadow(0);
    VDP_setTextPalette(0);
    VDP_setPaletteColor(0, 0x000000); // Black
    VDP_setPaletteColor(1, 0xF0);     // Green

    VDP_loadFont(custom_font.tileset, DMA); // Load the custom font ...
    // VDP_setPalette(PAL0, custom_font.palette->data); // ... and set the pallete from font file

    VDP_drawText("MEGA-CC OSP: VERSION 1.0", 0, 0);
    VDP_drawText("MEGA-CC RAM: ", 0, 1);

    VDP_drawText("INITING ...", 13, 1);
    if (!CC_RAM_Init())
        VDP_drawText("CHECKED    ", 13, 1);
    else
        VDP_drawText("WITH ERRORS", 13, 1);

    VDP_drawText("READY ... ", 0, 2);
    VDP_drawText("MEGA CODE CRACKER", 10, 7);
    VDP_drawText("OPEN SOURCE PROJECT", 9, 8);

    Print_Code_Table(posx, posy);

    VDP_drawText("USE D-PAD NAVIGATION AND NEXT KEYS FOR:", 0, 23);
    VDP_drawText("A - ABOVE, B - BELLOW, C - CHOOSE CHANGE", 0, 24);
    VDP_drawText("START - TO RUN PROGRAM FROM IN-SLOT CART", 0, 25);
    VDP_drawText("@ 2022 - WITH LOVE FROM RUSSIA BY MIGERA", 0, 27);

    // Program name in header
    /*
    char str[32];
    for (u8 i = 0; i < 32; i++) str[i] = *(u8 *)(0x150 + i);
    VDP_drawText(str, 0, 14);
    for (u8 i = 0; i < 32; i++) str[i] = *(u8 *)(0x400150 + i);
    VDP_drawText(str, 0, 15);
    */

    // Test address space
    /*
    address = 0x000100;
    datbuff = *address;
    printhex32z((u32)address, 6, 0, 4);
    printhex16(datbuff, 4, 9, 4);

    address = 0x100100;
    datbuff = *address;
    printhex32z((u32)address, 6, 0, 5);
    printhex16(datbuff, 4, 9, 5);

    address = 0x400100;
    datbuff = *address;
    printhex32z((u32)address, 6, 0, 6);
    printhex16(datbuff, 4, 9, 6);
    */

    while (TRUE)
    {
        if (!(tick++ % 10))
            mask = !mask; // revert every period ...

        // Print_Cursor_Position(); // for debug - print cursor position in korner

        Reprint_Cursor(); // cursor animation

        SYS_doVBlankProcess(); // always call this method at the end of the frame
    }

    return 0;
}

void ever_VInt()
{
    // SYS_doVBlankProcessEx(IMMEDIATELY);

    // SYS_disableInts();
    // Z80_requestBus(TRUE);

    // Put here anything ...

    // Z80_releaseBus();
    // SYS_enableInts();
}

void Print_Code_Table(u8 x, u8 y)
{
    for (u8 i = 0; i < maxcode; i++)
    {
        printhex16(tbladdr[i], 4, x, y + i);
        VDP_drawText(" : ", x + 4, y + i);
        printhex8(tbldata[i], 2, x + 7, y + i);
        VDP_drawText(" [", x + 9, y + i);
        if (tblstat[i] == 0)
            VDP_drawText(" ", x + 11, y + i);
        else
            VDP_drawText("*", x + 11, y + i);
        VDP_drawText("]", x + 12, y + i);
    }
}

void Print_Cursor_Position()
{
    printhex8(xm, 2, 36, 0);
    printhex8(ym, 2, 38, 0);
}

void Reprint_Cursor()
{
    if (mask)
    {
        if (xm < 4)
            VDP_drawText(" ", posx + xm, posy + ym);
        else
            VDP_drawText(" ", posx + xm + 3, posy + ym);
    }
    else
    {
        if (xm < 4)
            printhex16(tbladdr[ym], 4, posx, posy + ym);
        else
            printhex8(tbldata[ym], 2, posx + 7, posy + ym);
    }
}

u8 ccramrd(u16 addr)
{
    return (*(u8 *)(0x080000 + addr * 2 + 1));
}

void ccramwr(u16 addr, u8 data)
{
    *(u8 *)(0x080000 + addr * 2 + 1) = data;
}

u8 CC_RAM_Init()
{
    u8 err = 0;

    // Varian 1 - Fast but large code space and not buatifull
    /*
    u16 *baseaddr;
    baseaddr = 0x080000;
    for (u16 i = 0; i < 32768; i++)
        *(baseaddr + i) = 0x5A;
    for (u16 i = 0; i < 32768; i++)
        if ((*(baseaddr + i) & 0x00FF) != 0x5A)
            err++;

    if (err)
        return (err);

    for (u16 i = 0; i < 32768; i++)
        *(baseaddr + i) = 0;
    for (u16 i = 0; i < 32768; i++)
        if ((*(baseaddr + i) & 0x00FF) != 0)
            err++;
    */

    // Varian 2 - A little prettier, but maybe a little slower
    /*
    u8 *baseaddr;
    baseaddr = 0x080000;
    for (u16 i = 0; i < 32768; i++)
        *(baseaddr + (i * 2) + 1) = 0x5A;
    for (u16 i = 0; i < 32768; i++)
        if (*(baseaddr + (i * 2) + 1) != 0x5A)
            err++;

    if (err)
        return (err);

    for (u16 i = 0; i < 32768; i++)
        *(baseaddr + (i * 2) + 1) = 0x00;
    for (u16 i = 0; i < 32768; i++)
        if (*(baseaddr + (i * 2) + 1) != 0x00)
            err++;
    */

    // Variant 3- It looks not bad, but the speed may not be the best
    for (u16 i = 0; i < 32768; i++)
        ccramwr(i, 0x5A);
    for (u16 i = 0; i < 32768; i++)
        if (ccramrd(i) != 0x5A)
            err++;

    if (err)
        return (err);

    for (u16 i = 0; i < 32768; i++)
        ccramwr(i, 0);
    for (u16 i = 0; i < 32768; i++)
        if (ccramrd(i) != 0)
            err++;

    return (err);
}