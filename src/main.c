#include <genesis.h>
#include "ccram.h"
#include "ccrom.h"
#include "hexout.h"
#include "font.h"
#include "asm.h"

static void ever_VInt();
static void Print_Code_Table(u8 x, u8 y);
static void Print_Cursor_Position();
static void Reprint_Cursor();

// Params section

#define maxcode 10 // max quantity codes
#define posx 12    // offset on screen for print codetable
#define posy 9     // offset on screen for print codetable

// Declarations section - Global variable

volatile u16 tbladdr[maxcode]; // array of address part of "code"
volatile u8 tbldata[maxcode];  // array of data part of "code"
volatile u8 tblstat[maxcode];  // array of status part of "code" and (aka) primus byte of address

volatile u8 xm = 0; // x-position of menu cursor
volatile u8 ym = 0; // y-position of menu cursor

volatile u32 tick = 0;    // growing counter
volatile u8 mask = 0;     // time period flag
volatile u8 writable = 0; // flash-ROM presence flag

volatile u16 flashMfg;   // flash-ROM manufacturer code
volatile u16 flashIdent; // flash-ROM device codeF

volatile u16 *address; // pointer for data cell
volatile u8 *control;  // pointer for control reg. cell
volatile u8 status;    // variable for use taked status byte

volatile u16 datbuff; // for debug

void Joy_Handler(u16 joy, u16 changed, u16 state)
{
    if (joy == JOY_1)
    {
        if (writable & ((BUTTON_DIR) == (state & (BUTTON_DIR))))
        {
            VDP_drawText("WRITING ... ", 0, 4);
            Backup_Save();
            if (status & 0x20)
                VDP_drawText("ERASE ERROR!", 0, 4);
            else if (status & 0x10)
                VDP_drawText("WRITE ERROR!", 0, 4);
            else if (status & 0x08)
                VDP_drawText("VPP TOO LOW!", 0, 4);
            else
                VDP_drawText("OK!         ", 0, 4);
            Backup_Load();
            Print_Code_Table(posx, posy);
        }
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

        if ((BUTTON_A | BUTTON_B) == (state & (BUTTON_A | BUTTON_B)))
        {
            tbladdr[ym] = 0;
            tbldata[ym] = 0;
            tblstat[ym] = 0;
            Print_Code_Table(posx, posy);
        }
        else if (state & changed & BUTTON_A)
        {
            if (xm < 4)
                tbladdr[ym] = tbladdr[ym] + (0x1000 >> (xm * 4));
            else
                tbldata[ym] = tbldata[ym] + (0x10 >> ((xm - 4) * 4));
            tblstat[ym] = 0xFF;
            Print_Code_Table(posx, posy);
        }
        else if (state & changed & BUTTON_B)
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
            if (tblstat[ym] == 0x00) // set now 0xFF
            {
                tblstat[ym] = 0xFF;
                VDP_drawText("*", posx + 11, posy + ym);
            }
            else if (tblstat[ym] == 0xFF) // set now 0xFE
            {
                tblstat[ym] = 0xFE;
                VDP_drawText("!", posx + 11, posy + ym);
            }
            else if (tblstat[ym] == 0xFE) // set now 0x00
            {
                tblstat[ym] = 0x00;
                VDP_drawText(" ", posx + 11, posy + ym);
            }
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

            *(u8 *)(0x080001) = 200; // start delay on quantity of VInt, 1 - w/o delay

            u8 ramcnt = 1; // init value CC-RAM counter

            // Filling CC-RAM of actual codes
            for (u8 i = 0; i < maxcode; i++)
            {
                if (tblstat[i])
                {
                    CCRAM_u8_wr(ramcnt, tblstat[i]); // primus adrress byte - 0xFF if code actived
                    ramcnt++;
                    CCRAM_u16_wr(ramcnt, tbladdr[i]); // address word
                    ramcnt++;
                    ramcnt++;
                    CCRAM_u8_wr(ramcnt, tbldata[i]); // data byte
                    ramcnt++;
                }
            }

            // Setting runned in-slot Cart flag (last byte of Mega-CC RAM)
            *(u8 *)(0x08FFFF) = 0xCC; // see value in sega.s

            // Switch mapper to in-slot Cart
            *(u8 *)(0x3FFFFF) = 0xCC; // any random value

            // Switch mapper to in-slot cart - alt (stock) variant
            /*
            asm volatile("lea   	0x00FF0000, %a5");
            asm volatile("movea.l	%a5, %a0");
            asm volatile("move.w	#0x2639, (%a5)+");
            asm volatile("move.l	#0x0003F0000, (%a5)+");
            asm volatile("move.w	#0x4EF9, (%a5)+");
            asm volatile("move.l	#0x00FF0000, (%a5)+");
            asm volatile("jmp	    (%a0)");
            */
        }
    }
}

void __attribute__((noinline, used, longcall, section(".data"))) Backup_Save() // In-RAM function
{
    SYS_disableInts();
    Z80_requestBus(TRUE);

    address = 0x020000; // first address of BLOCK to erase|write
    control = 0x020001; // low part of this BLOCK for get status-flag

    *control = 0x50; // clear status bites
    *control = 0x20; // 1-st ctrl byte for erase init
    *control = 0xD0; // 2-nd ctrl byte for erase init
    status = *control;
    while ((status & 0x80) == 0) // wait until completed ...
        status = *control;
    *control = 0xFF;   // back to read-mode
    if (status & 0x08) // chech error ...
        return;        // ... Vpp is too low
    if (status & 0x20) // chech error ...
        return;        // ... Erase error

    for (u32 i = 0; i < maxcode; i++)
    {
        *(address + i * 2) = 0x0040; // ctrl byte for write word
        *(address + i * 2) = tbladdr[i];
        status = *control;
        while ((status & 0x80) == 0)
            status = *control;
        if (status & 0x08)
            break; // Vpp is too low
        if (status & 0x10)
            break; // Write error
    }
    *control = 0xFF;   // back to read-mode
    if (status & 0x08) // chech error ...
        return;        // ... Vpp is too low
    if (status & 0x10) // chech error ...
        return;        // ... Write error

    for (u32 i = 0; i < maxcode; i++)
    {
        *(maxcode * 2 + address + i * 2) = 0x0040; // ctrl byte for write word
        *(maxcode * 2 + address + i * 2) = tbldata[i];
        status = *control;
        while ((status & 0x80) == 0)
            status = *control;
        if (status & 0x08)
            break; // Vpp is too low
        if (status & 0x10)
            break; // Write error
    }
    *control = 0xFF;   // back to read-mode
    if (status & 0x08) // chech error ...
        return;        // ... Vpp is too low
    if (status & 0x10) // chech error ...
        return;        // ... Write error

    for (u32 i = 0; i < maxcode; i++)
    {
        *(maxcode * 4 + address + i * 2) = 0x0040; // ctrl byte for write word
        *(maxcode * 4 + address + i * 2) = tblstat[i];
        status = *control;
        while ((status & 0x80) == 0)
            status = *control;
        if (status & 0x08)
            break; // Vpp is too low
        if (status & 0x10)
            break; // Write error
    }
    *control = 0xFF;   // back to read-mode
    if (status & 0x08) // chech error ...
        return;        // ... Vpp is too low
    if (status & 0x10) // chech error ...
        return;        // ... Write error

    Z80_releaseBus();
    SYS_enableInts();
}

void Backup_Load()
{
    address = 0x020000;

    for (u32 i = 0; i < maxcode * 3; i++)
    {
        if (*(address + i * 2) != 0xFFFF) // Check area for cleanliness
        {
            // If exist data - load it
            for (u32 i = 0; i < maxcode; i++)
                tbladdr[i] = *(address + i * 2);
            for (u32 i = 0; i < maxcode; i++)
                tbldata[i] = (u8) * (maxcode * 2 + address + i * 2);
            for (u32 i = 0; i < maxcode; i++)
                tblstat[i] = (u8) * (maxcode * 4 + address + i * 2);
            break;
        }
    }
}

int main(bool hardReset)
{
    JOY_init();
    JOY_setEventHandler(&Joy_Handler);
    // SYS_setVIntCallback(ever_VInt);

    VDP_setScreenWidth320();
    VDP_setHInterrupt(0);
    VDP_setHilightShadow(0);
    VDP_setTextPalette(0);
    VDP_setPaletteColor(0, 0x000000); // Black
    VDP_setPaletteColor(1, 0xF0);     // Green

    VDP_loadFont(custom_font.tileset, DMA); // Load the custom font ...
    // VDP_setPalette(PAL0, custom_font.palette->data); // ... and set the pallete from font file

    VDP_drawText("MEGA-CC OSP: VERSION 1.3", 0, 0);
    VDP_drawText("MEGA-CC RAM: ", 0, 1);

    VDP_drawText("INITING ...", 13, 1);
    if (!CCRAM_Init(0xA5A5A5A5) && !CCRAM_Init(0x5A5A5A5A) && !CCRAM_Init(0x00000000))
        VDP_drawText("CHECKED    ", 13, 1);
    else
        VDP_drawText("WITH ERRORS", 13, 1);

    VDP_drawText("MEGA-CC ROM: ", 0, 2);

    flashMfg = CCROM_Info(0);
    flashIdent = CCROM_Info(2);

    if (flashMfg == 0x0089)
    {
        writable = 1;
        if (flashIdent == 0x4470)
            VDP_drawText("PA28F400-T", 13, 2);
        else if (flashIdent == 0x4471)
            VDP_drawText("PA28F400-B", 13, 2);
        else if (flashIdent == 0x2274)
            VDP_drawText("PA28F200-T", 13, 2);
        else if (flashIdent == 0x2275)
            VDP_drawText("PA28F200-B", 13, 2);
        else
            writable = 0;
    }
    else
    {
        writable = 0;
        VDP_drawText("UNSUPPORTED", 13, 2);
        printhex16(flashMfg, 4, 25, 2);
        printhex16(flashIdent, 4, 30, 2);
    }

    VDP_drawText("READY ... ", 0, 3);
    VDP_drawText("MEGA CODE CRACKER", 10, 6);
    VDP_drawText("OPEN SOURCE PROJECT", 9, 7);

    Backup_Load();
    Print_Code_Table(posx, posy);

    VDP_drawText("USE D-PAD NAVIGATION AND NEXT KEYS FOR:", 0, 20);
    VDP_drawText("A - ABOVE, B - BELLOW, C - CHOOSE CHANGE", 0, 22);
    VDP_drawText("START - TO RUN PROGRAM FROM IN-SLOT CART", 0, 23);
    if (writable)
    {
        VDP_drawText("CENTER D-PAD - TO SAVE CODE-TABLE IN R0M", 0, 24);
        VDP_drawText("A + B - TO CLEAN CURRENT STRING OF CODE", 0, 25);
    }
    else
        VDP_drawText("A + B - TO CLEAN CURRENT STRING OF CODE", 0, 24);
    VDP_drawText("@ 2022 - WITH LOVE FROM RUSSIA BY MIGERA", 0, 27);

    // Program name in header - debug feature
    /*
    char str[32];

    for (u8 i = 0; i < 32; i++) str[i] = *(u8 *)(0x150 + i);
    VDP_drawText(str, 0, 4);

    for (u8 i = 0; i < 32; i++) str[i] = *(u8 *)(0x400150 + i);
    VDP_drawText(str, 0, 5);
    */

    // Test address space
    /*
    VDP_drawText("DEBUG INFO: ", posx + 14, posy);

    address = 0x000100;
    datbuff = *address;
    printhex32z((u32)address, 6, posx + 14, posy + 2);
    printhex16(datbuff, 4, posx + 14 + 9, posy + 2);

    address = 0x100100;
    datbuff = *address;
    printhex32z((u32)address, 6, posx + 14, posy + 3);
    printhex16(datbuff, 4, posx + 14 + 9, posy + 3);

    address = 0x400100;
    datbuff = *address;
    printhex32z((u32)address, 6, posx + 14, posy + 4);
    printhex16(datbuff, 4, posx + 14 + 9, posy + 4);
    */

    while (TRUE)
    {
        if (!(tick++ % 10))
            mask = !mask; // revert every period ...

        // Print_Cursor_Position(); // for debug - print cursor position in korner
        Reprint_Cursor();      // cursor animation
        SYS_doVBlankProcess(); // always call this method at the end of the frame
    }

    return 0;
}

void ever_VInt()
{
    // SYS_doVBlankProcessEx(IMMEDIATELY);

    // SYS_disableInts();
    // Z80_requestBus(TRUE);

    // Put here anything ... if this activated ;-)

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
        else if (tblstat[i] == 0xFF)
            VDP_drawText("*", x + 11, y + i);
        else if (tblstat[i] == 0xFE)
            VDP_drawText("!", x + 11, y + i);
        VDP_drawText("]", x + 12, y + i);
    }
}

void Print_Cursor_Position() // debug feature
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
