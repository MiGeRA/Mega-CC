.section .text.keepboot

*-------------------------------------------------------
*
*       Sega startup code for the GNU Assembler
*       Translated from:
*       Sega startup code for the Sozobon C compiler
*       Written by Paul W. Lee
*       Modified by Charles Coty
*       Modified by Stephane Dallongeville
*
*-------------------------------------------------------

    .globl  rom_header

    .org    0x00000000

_Start_Of_Rom:
_Vecteurs_68K:
        dc.l    0x00000000              /* Stack address */
        dc.l    _Entry_Point            /* Program start address */
        dc.l    _Bus_Error
        dc.l    _Address_Error
        dc.l    _Illegal_Instruction
        dc.l    _Zero_Divide
        dc.l    _Chk_Instruction
        dc.l    _Trapv_Instruction
        dc.l    _Privilege_Violation
        dc.l    _Trace
        dc.l    _Line_1010_Emulation
        dc.l    _Line_1111_Emulation
        dc.l     _Error_Exception, _Error_Exception, _Error_Exception, _Error_Exception
        dc.l     _Error_Exception, _Error_Exception, _Error_Exception, _Error_Exception
        dc.l     _Error_Exception, _Error_Exception, _Error_Exception, _Error_Exception
        dc.l    _Error_Exception, _INT, _EXTINT, _INT
        dc.l    _HINT
        dc.l    0x00004EF9
        dc.l    _VINT
        dc.l    _INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT
        dc.l    _INT,_INT,_INT,_INT,_INT,_INT,_INT,_INT

rom_header:
        .incbin "out/rom_head.bin", 0x10, 0x100

_Entry_Point:
        move    #0x2700, %sr
        move.b	#0, 0x088FFF                /* cleaning flag */

        tst.l   0xa10008
        bne.s   SkipJoyDetect

        tst.w   0xa1000c

SkipJoyDetect:
        bne.s   SkipSetup

        lea     Table, %a5
        movem.w (%a5)+, %d5-%d7
        movem.l (%a5)+, %a0-%a4
* Check Version Number
        move.b  -0x10ff(%a1), %d0
        andi.b  #0x0f, %d0
        beq.s   WrongVersion

* Sega Security Code (SEGA)
        move.l  #0x53454741, 0x2f00(%a1)
WrongVersion:
        move.w  (%a4), %d0
        moveq   #0x00, %d0
        movea.l %d0, %a6
        move    %a6, %usp
        move.w  %d7, (%a1)
        move.w  %d7, (%a2)

* Jump to initialisation process now...

        jmp     _start_entry

SkipSetup:
        jmp     _reset_entry


Table:
        dc.w    0x8000,0x3fff,0x0100
        dc.l    0xA00000,0xA11100,0xA11200,0xC00000,0xC00004


*------------------------------------------------
*
*       interrupt functions
*
*------------------------------------------------

registersDump:
        move.l %d0, registerState+0
        move.l %d1, registerState+4
        move.l %d2, registerState+8
        move.l %d3, registerState+12
        move.l %d4, registerState+16
        move.l %d5, registerState+20
        move.l %d6, registerState+24
        move.l %d7, registerState+28
        move.l %a0, registerState+32
        move.l %a1, registerState+36
        move.l %a2, registerState+40
        move.l %a3, registerState+44
        move.l %a4, registerState+48
        move.l %a5, registerState+52
        move.l %a6, registerState+56
        move.l %a7, registerState+60
        rts

busAddressErrorDump:
        move.w 4(%sp), ext1State
        move.l 6(%sp), addrState
        move.w 10(%sp), ext2State
        move.w 12(%sp), srState
        move.l 14(%sp), pcState
        jmp registersDump

exception4WDump:
        move.w 4(%sp), srState
        move.l 6(%sp), pcState
        move.w 10(%sp), ext1State
        jmp registersDump

exceptionDump:
        move.w 4(%sp), srState
        move.l 6(%sp), pcState
        jmp registersDump


_Bus_Error:
        jsr busAddressErrorDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  busErrorCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Address_Error:
        jsr busAddressErrorDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  addressErrorCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Illegal_Instruction:
        jsr exception4WDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  illegalInstCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Zero_Divide:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  zeroDivideCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Chk_Instruction:
        jsr exception4WDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  chkInstCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Trapv_Instruction:
        jsr exception4WDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  trapvInstCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Privilege_Violation:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  privilegeViolationCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Trace:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  traceCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Line_1010_Emulation:
_Line_1111_Emulation:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  line1x1xCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_Error_Exception:
        jsr exceptionDump
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  errorExceptionCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_INT:
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  intCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_EXTINT:
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  eintCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_HINT:
        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move.l  hintCB, %a0
        jsr    (%a0)
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        rte

_VINT:
        cmpi.b  #0xCC, 0x088FFF
        bne.s   _next_init_cc

        movem.l %d0-%d1/%a0-%a1, -(%sp)
        move    %sr, %d1
        move    #0x2700, %sr

        move.w	#0x0100, 0xA11100                                
        move.w  #0x0100, 0xA11200           /* It too need! - for best compatibility with damn poor soft */                     
_wait_stop:
        btst.b	#0, 0xA11100                                    
        bne.s	_wait_stop
                                                         
        move.l  #0x080001, %a0
        subi.b  #1, (%a0)
        tst.b   (%a0)
        bne.s   _back_to_game
        addi.b  #1, (%a0)
        addq.l  #2, %a0
_next_code:
        moveq   #0, %d0
        move.b  (%a0), %d0
        addq.l  #2, %a0
        asl.l   #8, %d0
        move.b  (%a0), %d0
        addq.l  #2, %a0
        asl.l   #8, %d0
        move.b  (%a0), %d0
        addq.l  #2, %a0
        move.l  %d0, %a1
        move.b  (%a0), (%a1)
        addq.l  #2, %a0
        tst.b   (%a0)
        bne.s   _next_code
_back_to_game:       
        move.w  #0x000, 0xA11100
        move    %d1, %sr
        movem.l (%sp)+, %d0-%d1/%a0-%a1
        jmp     0x00000076

_next_init_cc:
        movem.l %d0-%d7/%a0-%a6, -(%sp)
        ori.w   #0x0001, intTrace           /* in V-Int */
        addq.l  #1, vtimer                  /* increment frame counter (more a vint counter) */
        btst    #3, VBlankProcess+1         /* PROCESS_XGM_TASK ? (use VBlankProcess+1 as btst is a byte operation) */
        beq.s   _no_xgm_task

        jsr     XGM_doVBlankProcess         /* do XGM vblank task */

_no_xgm_task:
        btst    #1, VBlankProcess+1         /* PROCESS_BITMAP_TASK ? (use VBlankProcess+1 as btst is a byte operation) */
        beq.s   _no_bmp_task

        jsr     BMP_doVBlankProcess         /* do BMP vblank task */

_no_bmp_task:
        move.l  vintCB, %a0                 /* load user callback */
        jsr    (%a0)                        /* call user callback */
        andi.w  #0xFFFE, intTrace           /* out V-Int */
        movem.l (%sp)+, %d0-%d7/%a0-%a6
        rte
