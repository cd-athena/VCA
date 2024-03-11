;*****************************************************************************
;* Copyright (C) 2024 Christian Doppler Laboratory ATHENA
;*
;* Authors: Nabajit Deka <nabajit@multicorewareinc.com>
;*          Min Chen <chenm003@163.com>
;*          Li Cao <li@multicorewareinc.com>
;*          Praveen Kumar Tiwari <Praveen@multicorewareinc.com>
;*
;* This program is free software; you can redistribute it and/or modify
;* it under the terms of the GNU General Public License as published by
;* the Free Software Foundation; either version 3 of the License, or
;* (at your option) any later version.
;*
;* This program is distributed in the hope that it will be useful,
;* but WITHOUT ANY WARRANTY; without even the implied warranty of
;* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;* GNU General Public License for more details.
;*
;* You should have received a copy of the GNU General Public License
;* along with this program.
;*****************************************************************************/

;TO-DO : Further optimize the routines.

%include "x86inc.asm"
%include "x86util.asm"
SECTION_RODATA 64

dct8_shuf:         times 2 db 6, 7, 4, 5, 2, 3, 0, 1, 14, 15, 12, 13, 10, 11, 8, 9

tab_dct8:       dw 64, 64, 64, 64, 64, 64, 64, 64
                dw 89, 75, 50, 18, -18, -50, -75, -89
                dw 83, 36, -36, -83, -83, -36, 36, 83
                dw 75, -18, -89, -50, 50, 89, 18, -75
                dw 64, -64, -64, 64, 64, -64, -64, 64
                dw 50, -89, 18, 75, -75, -18, 89, -50
                dw 36, -83, 83, -36, -36, 83, -83, 36
                dw 18, -50, 75, -89, 89, -75, 50, -18

tab_dct16_1:    dw 64, 64, 64, 64, 64, 64, 64, 64
                dw 90, 87, 80, 70, 57, 43, 25,  9
                dw 89, 75, 50, 18, -18, -50, -75, -89
                dw 87, 57,  9, -43, -80, -90, -70, -25
                dw 83, 36, -36, -83, -83, -36, 36, 83
                dw 80,  9, -70, -87, -25, 57, 90, 43
                dw 75, -18, -89, -50, 50, 89, 18, -75
                dw 70, -43, -87,  9, 90, 25, -80, -57
                dw 64, -64, -64, 64, 64, -64, -64, 64
                dw 57, -80, -25, 90, -9, -87, 43, 70
                dw 50, -89, 18, 75, -75, -18, 89, -50
                dw 43, -90, 57, 25, -87, 70,  9, -80
                dw 36, -83, 83, -36, -36, 83, -83, 36
                dw 25, -70, 90, -80, 43,  9, -57, 87
                dw 18, -50, 75, -89, 89, -75, 50, -18
                dw  9, -25, 43, -57, 70, -80, 87, -90

tab_dct16_2:    dw 64, 64, 64, 64, 64, 64, 64, 64
                dw -9, -25, -43, -57, -70, -80, -87, -90
                dw -89, -75, -50, -18, 18, 50, 75, 89
                dw 25, 70, 90, 80, 43, -9, -57, -87
                dw 83, 36, -36, -83, -83, -36, 36, 83
                dw -43, -90, -57, 25, 87, 70, -9, -80
                dw -75, 18, 89, 50, -50, -89, -18, 75
                dw 57, 80, -25, -90, -9, 87, 43, -70
                dw 64, -64, -64, 64, 64, -64, -64, 64
                dw -70, -43, 87,  9, -90, 25, 80, -57
                dw -50, 89, -18, -75, 75, 18, -89, 50
                dw 80, -9, -70, 87, -25, -57, 90, -43
                dw 36, -83, 83, -36, -36, 83, -83, 36
                dw -87, 57, -9, -43, 80, -90, 70, -25
                dw -18, 50, -75, 89, -89, 75, -50, 18
                dw 90, -87, 80, -70, 57, -43, 25, -9

dct16_shuf1:     times 2 db 14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1

dct16_shuf2:    times 2 db 0, 1, 14, 15, 2, 3, 12, 13, 4, 5, 10, 11, 6, 7, 8, 9

tab_dct32_1:    dw 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
                dw 90, 90, 88, 85, 82, 78, 73, 67, 61, 54, 46, 38, 31, 22, 13,  4
                dw 90, 87, 80, 70, 57, 43, 25,  9, -9, -25, -43, -57, -70, -80, -87, -90
                dw 90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13
                dw 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89
                dw 88, 67, 31, -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85, 61, 22
                dw 87, 57,  9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87
                dw 85, 46, -13, -67, -90, -73, -22, 38, 82, 88, 54, -4, -61, -90, -78, -31
                dw 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83
                dw 82, 22, -54, -90, -61, 13, 78, 85, 31, -46, -90, -67,  4, 73, 88, 38
                dw 80,  9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80
                dw 78, -4, -82, -73, 13, 85, 67, -22, -88, -61, 31, 90, 54, -38, -90, -46
                dw 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75
                dw 73, -31, -90, -22, 78, 67, -38, -90, -13, 82, 61, -46, -88, -4, 85, 54
                dw 70, -43, -87,  9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70
                dw 67, -54, -78, 38, 85, -22, -90,  4, 90, 13, -88, -31, 82, 46, -73, -61
                dw 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64
                dw 61, -73, -46, 82, 31, -88, -13, 90, -4, -90, 22, 85, -38, -78, 54, 67
                dw 57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87,  9, -90, 25, 80, -57
                dw 54, -85, -4, 88, -46, -61, 82, 13, -90, 38, 67, -78, -22, 90, -31, -73
                dw 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50
                dw 46, -90, 38, 54, -90, 31, 61, -88, 22, 67, -85, 13, 73, -82,  4, 78
                dw 43, -90, 57, 25, -87, 70,  9, -80, 80, -9, -70, 87, -25, -57, 90, -43
                dw 38, -88, 73, -4, -67, 90, -46, -31, 85, -78, 13, 61, -90, 54, 22, -82
                dw 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36
                dw 31, -78, 90, -61,  4, 54, -88, 82, -38, -22, 73, -90, 67, -13, -46, 85
                dw 25, -70, 90, -80, 43,  9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25
                dw 22, -61, 85, -90, 73, -38, -4, 46, -78, 90, -82, 54, -13, -31, 67, -88
                dw 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18
                dw 13, -38, 61, -78, 88, -90, 85, -73, 54, -31,  4, 22, -46, 67, -82, 90
                dw 9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9
                dw 4, -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90

tab_dct32_2:    dw 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
                dw -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90
                dw -90, -87, -80, -70, -57, -43, -25, -9,  9, 25, 43, 57, 70, 80, 87, 90
                dw 13, 38, 61, 78, 88, 90, 85, 73, 54, 31,  4, -22, -46, -67, -82, -90
                dw 89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89
                dw -22, -61, -85, -90, -73, -38,  4, 46, 78, 90, 82, 54, 13, -31, -67, -88
                dw -87, -57, -9, 43, 80, 90, 70, 25, -25, -70, -90, -80, -43,  9, 57, 87
                dw 31, 78, 90, 61,  4, -54, -88, -82, -38, 22, 73, 90, 67, 13, -46, -85
                dw 83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83
                dw -38, -88, -73, -4, 67, 90, 46, -31, -85, -78, -13, 61, 90, 54, -22, -82
                dw -80, -9, 70, 87, 25, -57, -90, -43, 43, 90, 57, -25, -87, -70,  9, 80
                dw 46, 90, 38, -54, -90, -31, 61, 88, 22, -67, -85, -13, 73, 82,  4, -78
                dw 75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75
                dw -54, -85,  4, 88, 46, -61, -82, 13, 90, 38, -67, -78, 22, 90, 31, -73
                dw -70, 43, 87, -9, -90, -25, 80, 57, -57, -80, 25, 90,  9, -87, -43, 70
                dw 61, 73, -46, -82, 31, 88, -13, -90, -4, 90, 22, -85, -38, 78, 54, -67
                dw 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64
                dw -67, -54, 78, 38, -85, -22, 90,  4, -90, 13, 88, -31, -82, 46, 73, -61
                dw -57, 80, 25, -90,  9, 87, -43, -70, 70, 43, -87, -9, 90, -25, -80, 57
                dw 73, 31, -90, 22, 78, -67, -38, 90, -13, -82, 61, 46, -88,  4, 85, -54
                dw 50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50
                dw -78, -4, 82, -73, -13, 85, -67, -22, 88, -61, -31, 90, -54, -38, 90, -46
                dw -43, 90, -57, -25, 87, -70, -9, 80, -80,  9, 70, -87, 25, 57, -90, 43
                dw 82, -22, -54, 90, -61, -13, 78, -85, 31, 46, -90, 67,  4, -73, 88, -38
                dw 36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36
                dw -85, 46, 13, -67, 90, -73, 22, 38, -82, 88, -54, -4, 61, -90, 78, -31
                dw -25, 70, -90, 80, -43, -9, 57, -87, 87, -57,  9, 43, -80, 90, -70, 25
                dw 88, -67, 31, 13, -54, 82, -90, 78, -46,  4, 38, -73, 90, -85, 61, -22
                dw 18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18
                dw -90, 82, -67, 46, -22, -4, 31, -54, 73, -85, 90, -88, 78, -61, 38, -13
                dw -9, 25, -43, 57, -70, 80, -87, 90, -90, 87, -80, 70, -57, 43, -25,  9
                dw 90, -90, 88, -85, 82, -78, 73, -67, 61, -54, 46, -38, 31, -22, 13, -4

tab_dct4:       times 4 dw 64, 64
                times 4 dw 83, 36
                times 4 dw 64, -64
                times 4 dw 36, -83

tab_dct8_1:     times 2 dw 89, 50, 75, 18
                times 2 dw 75, -89, -18, -50
                times 2 dw 50, 18, -89, 75
                times 2 dw 18, 75, -50, -89

tab_dct8_2:     times 2 dd 83, 36
                times 2 dd 36, 83
                times 1 dd 89, 75, 50, 18
                times 1 dd 75, -18, -89, -50
                times 1 dd 50, -89, 18, 75
                times 1 dd 18, -50, 75, -89

pb_unpackhlw1:  db 0,1,8,9,2,3,10,11,4,5,12,13,6,7,14,15

SECTION .text
cextern pd_1
cextern pd_2
cextern pd_4
cextern pd_8
cextern pd_16
cextern pd_32
cextern pd_64
cextern pd_128
cextern pd_256
cextern pd_512
cextern pd_1024
cextern pd_2048
cextern pw_ppppmmmm
cextern trans8_shuf


%if BIT_DEPTH == 12
    %define     DCT4_SHIFT          5
    %define     DCT4_ROUND          16
    %define    IDCT_SHIFT           8
    %define    IDCT_ROUND           128
    %define     DST4_SHIFT          5
    %define     DST4_ROUND          16
    %define     DCT8_SHIFT1         6
    %define     DCT8_ROUND1         32
%elif BIT_DEPTH == 10
    %define     DCT4_SHIFT          3
    %define     DCT4_ROUND          4
    %define    IDCT_SHIFT           10
    %define    IDCT_ROUND           512
    %define     DST4_SHIFT          3
    %define     DST4_ROUND          4
    %define     DCT8_SHIFT1         4
    %define     DCT8_ROUND1         8
%elif BIT_DEPTH == 8
    %define     DCT4_SHIFT          1
    %define     DCT4_ROUND          1
    %define    IDCT_SHIFT           12
    %define    IDCT_ROUND           2048
    %define     DST4_SHIFT          1
    %define     DST4_ROUND          1
    %define     DCT8_SHIFT1         2
    %define     DCT8_ROUND1         2
%else
    %error Unsupported BIT_DEPTH!
%endif

%define         DCT8_ROUND2         256
%define         DCT8_SHIFT2         9

;-------------------------------------------------------
; void dct8(const int16_t* src, int16_t* dst, intptr_t srcStride)
;-------------------------------------------------------
INIT_XMM sse2
%if BIT_DEPTH == 12
cglobal dct8_12bit, 3,6,8,0-16*mmsize
%elif BIT_DEPTH == 10
cglobal dct8_10bit, 3,6,8,0-16*mmsize
%elif BIT_DEPTH == 8
cglobal dct8_8bit, 3,6,8,0-16*mmsize
%else
    %error Unsupported BIT_DEPTH!
%endif
    ;------------------------
    ; Stack Mapping(dword)
    ;------------------------
    ; Row0[0-3] Row1[0-3]
    ; ...
    ; Row6[0-3] Row7[0-3]
    ; Row0[0-3] Row7[0-3]
    ; ...
    ; Row6[4-7] Row7[4-7]
    ;------------------------

    add         r2, r2
    lea         r3, [r2 * 3]
    mov         r5, rsp
%assign x 0
%rep 2
    movu        m0, [r0]
    movu        m1, [r0 + r2]
    movu        m2, [r0 + r2 * 2]
    movu        m3, [r0 + r3]

    punpcklwd   m4, m0, m1
    punpckhwd   m0, m1
    punpcklwd   m5, m2, m3
    punpckhwd   m2, m3
    punpckldq   m1, m4, m5          ; m1 = [1 0]
    punpckhdq   m4, m5              ; m4 = [3 2]
    punpckldq   m3, m0, m2
    punpckhdq   m0, m2
    pshufd      m2, m3, 0x4E        ; m2 = [4 5]
    pshufd      m0, m0, 0x4E        ; m0 = [6 7]

    paddw       m3, m1, m0
    psubw       m1, m0              ; m1 = [d1 d0]
    paddw       m0, m4, m2
    psubw       m4, m2              ; m4 = [d3 d2]
    punpcklqdq  m2, m3, m0          ; m2 = [s2 s0]
    punpckhqdq  m3, m0
    pshufd      m3, m3, 0x4E        ; m3 = [s1 s3]

    punpcklwd   m0, m1, m4          ; m0 = [d2/d0]
    punpckhwd   m1, m4              ; m1 = [d3/d1]
    punpckldq   m4, m0, m1          ; m4 = [d3 d1 d2 d0]
    punpckhdq   m0, m1              ; m0 = [d3 d1 d2 d0]

    ; odd
    lea         r4, [tab_dct8_1]
    pmaddwd     m1, m4, [r4 + 0*16]
    pmaddwd     m5, m0, [r4 + 0*16]
    pshufd      m1, m1, 0xD8
    pshufd      m5, m5, 0xD8
    mova        m7, m1
    punpckhqdq  m7, m5
    punpcklqdq  m1, m5
    paddd       m1, m7
    paddd       m1, [pd_ %+ DCT8_ROUND1]
    psrad       m1, DCT8_SHIFT1
  %if x == 1
    pshufd      m1, m1, 0x1B
  %endif
    mova        [r5 + 1*2*mmsize], m1 ; Row 1

    pmaddwd     m1, m4, [r4 + 1*16]
    pmaddwd     m5, m0, [r4 + 1*16]
    pshufd      m1, m1, 0xD8
    pshufd      m5, m5, 0xD8
    mova        m7, m1
    punpckhqdq  m7, m5
    punpcklqdq  m1, m5
    paddd       m1, m7
    paddd       m1, [pd_ %+ DCT8_ROUND1]
    psrad       m1, DCT8_SHIFT1
  %if x == 1
    pshufd      m1, m1, 0x1B
  %endif
    mova        [r5 + 3*2*mmsize], m1 ; Row 3

    pmaddwd     m1, m4, [r4 + 2*16]
    pmaddwd     m5, m0, [r4 + 2*16]
    pshufd      m1, m1, 0xD8
    pshufd      m5, m5, 0xD8
    mova        m7, m1
    punpckhqdq  m7, m5
    punpcklqdq  m1, m5
    paddd       m1, m7
    paddd       m1, [pd_ %+ DCT8_ROUND1]
    psrad       m1, DCT8_SHIFT1
  %if x == 1
    pshufd      m1, m1, 0x1B
  %endif
    mova        [r5 + 5*2*mmsize], m1 ; Row 5

    pmaddwd     m4, [r4 + 3*16]
    pmaddwd     m0, [r4 + 3*16]
    pshufd      m4, m4, 0xD8
    pshufd      m0, m0, 0xD8
    mova        m7, m4
    punpckhqdq  m7, m0
    punpcklqdq  m4, m0
    paddd       m4, m7
    paddd       m4, [pd_ %+ DCT8_ROUND1]
    psrad       m4, DCT8_SHIFT1
  %if x == 1
    pshufd      m4, m4, 0x1B
  %endif
    mova        [r5 + 7*2*mmsize], m4; Row 7

    ; even
    lea         r4, [tab_dct4]
    paddw       m0, m2, m3          ; m0 = [EE1 EE0]
    pshufd      m0, m0, 0xD8
    pshuflw     m0, m0, 0xD8
    pshufhw     m0, m0, 0xD8
    psubw       m2, m3              ; m2 = [EO1 EO0]
    pmullw      m2, [pw_ppppmmmm]
    pshufd      m2, m2, 0xD8
    pshuflw     m2, m2, 0xD8
    pshufhw     m2, m2, 0xD8
    pmaddwd     m3, m0, [r4 + 0*16]
    paddd       m3, [pd_ %+ DCT8_ROUND1]
    psrad       m3, DCT8_SHIFT1
  %if x == 1
    pshufd      m3, m3, 0x1B
  %endif
    mova        [r5 + 0*2*mmsize], m3 ; Row 0
    pmaddwd     m0, [r4 + 2*16]
    paddd       m0, [pd_ %+ DCT8_ROUND1]
    psrad       m0, DCT8_SHIFT1
  %if x == 1
    pshufd      m0, m0, 0x1B
  %endif
    mova        [r5 + 4*2*mmsize], m0 ; Row 4
    pmaddwd     m3, m2, [r4 + 1*16]
    paddd       m3, [pd_ %+ DCT8_ROUND1]
    psrad       m3, DCT8_SHIFT1
  %if x == 1
    pshufd      m3, m3, 0x1B
  %endif
    mova        [r5 + 2*2*mmsize], m3 ; Row 2
    pmaddwd     m2, [r4 + 3*16]
    paddd       m2, [pd_ %+ DCT8_ROUND1]
    psrad       m2, DCT8_SHIFT1
  %if x == 1
    pshufd      m2, m2, 0x1B
  %endif
    mova        [r5 + 6*2*mmsize], m2 ; Row 6

  %if x != 1
    lea         r0, [r0 + r2 * 4]
    add         r5, mmsize
  %endif
%assign x x+1
%endrep

    mov         r0, rsp                 ; r0 = pointer to Low Part
    lea         r4, [tab_dct8_2]

%assign x 0
%rep 4
    mova        m0, [r0 + 0*2*mmsize]     ; [3 2 1 0]
    mova        m1, [r0 + 1*2*mmsize]
    paddd       m2, m0, [r0 + (0*2+1)*mmsize]
    pshufd      m2, m2, 0x9C            ; m2 = [s2 s1 s3 s0]
    paddd       m3, m1, [r0 + (1*2+1)*mmsize]
    pshufd      m3, m3, 0x9C            ; m3 = ^^
    psubd       m0, [r0 + (0*2+1)*mmsize]     ; m0 = [d3 d2 d1 d0]
    psubd       m1, [r0 + (1*2+1)*mmsize]     ; m1 = ^^

    ; even
    pshufd      m4, m2, 0xD8
    pshufd      m3, m3, 0xD8
    mova        m7, m4
    punpckhqdq  m7, m3
    punpcklqdq  m4, m3
    mova        m2, m4
    paddd       m4, m7                  ; m4 = [EE1 EE0 EE1 EE0]
    psubd       m2, m7                  ; m2 = [EO1 EO0 EO1 EO0]

    pslld       m4, 6                   ; m4 = [64*EE1 64*EE0]
    mova        m5, m2
    pmuludq     m5, [r4 + 0*16]
    pshufd      m7, m2, 0xF5
    movu        m6, [r4 + 0*16 + 4]
    pmuludq     m7, m6
    pshufd      m5, m5, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m5, m7                  ; m5 = [36*EO1 83*EO0]
    pshufd      m7, m2, 0xF5
    pmuludq     m2, [r4 + 1*16]
    movu        m6, [r4 + 1*16 + 4]
    pmuludq     m7, m6
    pshufd      m2, m2, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m2, m7                  ; m2 = [83*EO1 36*EO0]

    pshufd      m3, m4, 0xD8
    pshufd      m5, m5, 0xD8
    mova        m7, m3
    punpckhqdq  m7, m5
    punpcklqdq  m3, m5
    paddd       m3, m7                  ; m3 = [Row2 Row0]
    paddd       m3, [pd_ %+ DCT8_ROUND2]
    psrad       m3, DCT8_SHIFT2
    pshufd      m4, m4, 0xD8
    pshufd      m2, m2, 0xD8
    mova        m7, m4
    punpckhqdq  m7, m2
    punpcklqdq  m4, m2
    psubd       m4, m7                  ; m4 = [Row6 Row4]
    paddd       m4, [pd_ %+ DCT8_ROUND2]
    psrad       m4, DCT8_SHIFT2

    packssdw    m3, m3
    movd        [r1 + 0*mmsize], m3
    pshufd      m3, m3, 1
    movd        [r1 + 2*mmsize], m3

    packssdw    m4, m4
    movd        [r1 + 4*mmsize], m4
    pshufd      m4, m4, 1
    movd        [r1 + 6*mmsize], m4

    ; odd
    mova        m2, m0
    pmuludq     m2, [r4 + 2*16]
    pshufd      m7, m0, 0xF5
    movu        m6, [r4 + 2*16 + 4]
    pmuludq     m7, m6
    pshufd      m2, m2, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m2, m7
    mova        m3, m1
    pmuludq     m3, [r4 + 2*16]
    pshufd      m7, m1, 0xF5
    pmuludq     m7, m6
    pshufd      m3, m3, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m3, m7
    mova        m4, m0
    pmuludq     m4, [r4 + 3*16]
    pshufd      m7, m0, 0xF5
    movu        m6, [r4 + 3*16 + 4]
    pmuludq     m7, m6
    pshufd      m4, m4, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m4, m7
    mova        m5, m1
    pmuludq     m5, [r4 + 3*16]
    pshufd      m7, m1, 0xF5
    pmuludq     m7, m6
    pshufd      m5, m5, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m5, m7
    pshufd      m2, m2, 0xD8
    pshufd      m3, m3, 0xD8
    mova        m7, m2
    punpckhqdq  m7, m3
    punpcklqdq  m2, m3
    paddd       m2, m7
    pshufd      m4, m4, 0xD8
    pshufd      m5, m5, 0xD8
    mova        m7, m4
    punpckhqdq  m7, m5
    punpcklqdq  m4, m5
    paddd       m4, m7
    pshufd      m2, m2, 0xD8
    pshufd      m4, m4, 0xD8
    mova        m7, m2
    punpckhqdq  m7, m4
    punpcklqdq  m2, m4
    paddd       m2, m7                  ; m2 = [Row3 Row1]
    paddd       m2, [pd_ %+ DCT8_ROUND2]
    psrad       m2, DCT8_SHIFT2

    packssdw    m2, m2
    movd        [r1 + 1*mmsize], m2
    pshufd      m2, m2, 1
    movd        [r1 + 3*mmsize], m2

    mova        m2, m0
    pmuludq     m2, [r4 + 4*16]
    pshufd      m7, m0, 0xF5
    movu        m6, [r4 + 4*16 + 4]
    pmuludq     m7, m6
    pshufd      m2, m2, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m2, m7
    mova        m3, m1
    pmuludq     m3, [r4 + 4*16]
    pshufd      m7, m1, 0xF5
    pmuludq     m7, m6
    pshufd      m3, m3, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m3, m7
    mova        m4, m0
    pmuludq     m4, [r4 + 5*16]
    pshufd      m7, m0, 0xF5
    movu        m6, [r4 + 5*16 + 4]
    pmuludq     m7, m6
    pshufd      m4, m4, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m4, m7
    mova        m5, m1
    pmuludq     m5, [r4 + 5*16]
    pshufd      m7, m1, 0xF5
    pmuludq     m7, m6
    pshufd      m5, m5, 0x88
    pshufd      m7, m7, 0x88
    punpckldq   m5, m7
    pshufd      m2, m2, 0xD8
    pshufd      m3, m3, 0xD8
    mova        m7, m2
    punpckhqdq  m7, m3
    punpcklqdq  m2, m3
    paddd       m2, m7
    pshufd      m4, m4, 0xD8
    pshufd      m5, m5, 0xD8
    mova        m7, m4
    punpckhqdq  m7, m5
    punpcklqdq  m4, m5
    paddd       m4, m7
    pshufd      m2, m2, 0xD8
    pshufd      m4, m4, 0xD8
    mova        m7, m2
    punpckhqdq  m7, m4
    punpcklqdq  m2, m4
    paddd       m2, m7                  ; m2 = [Row7 Row5]
    paddd       m2, [pd_ %+ DCT8_ROUND2]
    psrad       m2, DCT8_SHIFT2

    packssdw    m2, m2
    movd        [r1 + 5*mmsize], m2
    pshufd      m2, m2, 1
    movd        [r1 + 7*mmsize], m2
%if x < 3
    add         r1, mmsize/4
    add         r0, 2*2*mmsize
%endif
%assign x x+1
%endrep

    RET

;-------------------------------------------------------
; void dct8(const int16_t* src, int16_t* dst, intptr_t srcStride)
;-------------------------------------------------------
INIT_XMM sse4
%if BIT_DEPTH == 12
cglobal dct8_12bit, 3,6,7,0-16*mmsize
%elif BIT_DEPTH == 10
cglobal dct8_10bit, 3,6,7,0-16*mmsize
%elif BIT_DEPTH == 8
cglobal dct8_8bit, 3,6,7,0-16*mmsize
%else
    %error Unsupported BIT_DEPTH!
%endif
    ;------------------------
    ; Stack Mapping(dword)
    ;------------------------
    ; Row0[0-3] Row1[0-3]
    ; ...
    ; Row6[0-3] Row7[0-3]
    ; Row0[0-3] Row7[0-3]
    ; ...
    ; Row6[4-7] Row7[4-7]
    ;------------------------
    mova        m6, [pd_ %+ DCT8_ROUND1]

    add         r2, r2
    lea         r3, [r2 * 3]
    mov         r5, rsp
%assign x 0
%rep 2
    movu        m0, [r0]
    movu        m1, [r0 + r2]
    movu        m2, [r0 + r2 * 2]
    movu        m3, [r0 + r3]

    punpcklwd   m4, m0, m1
    punpckhwd   m0, m1
    punpcklwd   m5, m2, m3
    punpckhwd   m2, m3
    punpckldq   m1, m4, m5          ; m1 = [1 0]
    punpckhdq   m4, m5              ; m4 = [3 2]
    punpckldq   m3, m0, m2
    punpckhdq   m0, m2
    pshufd      m2, m3, 0x4E        ; m2 = [4 5]
    pshufd      m0, m0, 0x4E        ; m0 = [6 7]

    paddw       m3, m1, m0
    psubw       m1, m0              ; m1 = [d1 d0]
    paddw       m0, m4, m2
    psubw       m4, m2              ; m4 = [d3 d2]
    punpcklqdq  m2, m3, m0          ; m2 = [s2 s0]
    punpckhqdq  m3, m0
    pshufd      m3, m3, 0x4E        ; m3 = [s1 s3]

    punpcklwd   m0, m1, m4          ; m0 = [d2/d0]
    punpckhwd   m1, m4              ; m1 = [d3/d1]
    punpckldq   m4, m0, m1          ; m4 = [d3 d1 d2 d0]
    punpckhdq   m0, m1              ; m0 = [d3 d1 d2 d0]

    ; odd
    lea         r4, [tab_dct8_1]
    pmaddwd     m1, m4, [r4 + 0*16]
    pmaddwd     m5, m0, [r4 + 0*16]
    phaddd      m1, m5
    paddd       m1, m6
    psrad       m1, DCT8_SHIFT1
  %if x == 1
    pshufd      m1, m1, 0x1B
  %endif
    mova        [r5 + 1*2*mmsize], m1 ; Row 1

    pmaddwd     m1, m4, [r4 + 1*16]
    pmaddwd     m5, m0, [r4 + 1*16]
    phaddd      m1, m5
    paddd       m1, m6
    psrad       m1, DCT8_SHIFT1
  %if x == 1
    pshufd      m1, m1, 0x1B
  %endif
    mova        [r5 + 3*2*mmsize], m1 ; Row 3

    pmaddwd     m1, m4, [r4 + 2*16]
    pmaddwd     m5, m0, [r4 + 2*16]
    phaddd      m1, m5
    paddd       m1, m6
    psrad       m1, DCT8_SHIFT1
  %if x == 1
    pshufd      m1, m1, 0x1B
  %endif
    mova        [r5 + 5*2*mmsize], m1 ; Row 5

    pmaddwd     m4, [r4 + 3*16]
    pmaddwd     m0, [r4 + 3*16]
    phaddd      m4, m0
    paddd       m4, m6
    psrad       m4, DCT8_SHIFT1
  %if x == 1
    pshufd      m4, m4, 0x1B
  %endif
    mova        [r5 + 7*2*mmsize], m4; Row 7

    ; even
    lea         r4, [tab_dct4]
    paddw       m0, m2, m3          ; m0 = [EE1 EE0]
    pshufb      m0, [pb_unpackhlw1]
    psubw       m2, m3              ; m2 = [EO1 EO0]
    psignw      m2, [pw_ppppmmmm]
    pshufb      m2, [pb_unpackhlw1]
    pmaddwd     m3, m0, [r4 + 0*16]
    paddd       m3, m6
    psrad       m3, DCT8_SHIFT1
  %if x == 1
    pshufd      m3, m3, 0x1B
  %endif
    mova        [r5 + 0*2*mmsize], m3 ; Row 0
    pmaddwd     m0, [r4 + 2*16]
    paddd       m0, m6
    psrad       m0, DCT8_SHIFT1
  %if x == 1
    pshufd      m0, m0, 0x1B
  %endif
    mova        [r5 + 4*2*mmsize], m0 ; Row 4
    pmaddwd     m3, m2, [r4 + 1*16]
    paddd       m3, m6
    psrad       m3, DCT8_SHIFT1
  %if x == 1
    pshufd      m3, m3, 0x1B
  %endif
    mova        [r5 + 2*2*mmsize], m3 ; Row 2
    pmaddwd     m2, [r4 + 3*16]
    paddd       m2, m6
    psrad       m2, DCT8_SHIFT1
  %if x == 1
    pshufd      m2, m2, 0x1B
  %endif
    mova        [r5 + 6*2*mmsize], m2 ; Row 6

  %if x != 1
    lea         r0, [r0 + r2 * 4]
    add         r5, mmsize
  %endif
%assign x x+1
%endrep

    mov         r2, 2
    mov         r0, rsp                 ; r0 = pointer to Low Part
    lea         r4, [tab_dct8_2]
    mova        m6, [pd_256]

.pass2:
%rep 2
    mova        m0, [r0 + 0*2*mmsize]     ; [3 2 1 0]
    mova        m1, [r0 + 1*2*mmsize]
    paddd       m2, m0, [r0 + (0*2+1)*mmsize]
    pshufd      m2, m2, 0x9C            ; m2 = [s2 s1 s3 s0]
    paddd       m3, m1, [r0 + (1*2+1)*mmsize]
    pshufd      m3, m3, 0x9C            ; m3 = ^^
    psubd       m0, [r0 + (0*2+1)*mmsize]     ; m0 = [d3 d2 d1 d0]
    psubd       m1, [r0 + (1*2+1)*mmsize]     ; m1 = ^^

    ; even
    phaddd      m4, m2, m3              ; m4 = [EE1 EE0 EE1 EE0]
    phsubd      m2, m3                  ; m2 = [EO1 EO0 EO1 EO0]

    pslld       m4, 6                   ; m4 = [64*EE1 64*EE0]
    pmulld      m5, m2, [r4 + 0*16]     ; m5 = [36*EO1 83*EO0]
    pmulld      m2, [r4 + 1*16]         ; m2 = [83*EO1 36*EO0]

    phaddd      m3, m4, m5              ; m3 = [Row2 Row0]
    paddd       m3, m6
    psrad       m3, 9
    phsubd      m4, m2                  ; m4 = [Row6 Row4]
    paddd       m4, m6
    psrad       m4, 9

    packssdw    m3, m3
    movd        [r1 + 0*mmsize], m3
    pshufd      m3, m3, 1
    movd        [r1 + 2*mmsize], m3

    packssdw    m4, m4
    movd        [r1 + 4*mmsize], m4
    pshufd      m4, m4, 1
    movd        [r1 + 6*mmsize], m4

    ; odd
    pmulld      m2, m0, [r4 + 2*16]
    pmulld      m3, m1, [r4 + 2*16]
    pmulld      m4, m0, [r4 + 3*16]
    pmulld      m5, m1, [r4 + 3*16]
    phaddd      m2, m3
    phaddd      m4, m5
    phaddd      m2, m4                  ; m2 = [Row3 Row1]
    paddd       m2, m6
    psrad       m2, 9

    packssdw    m2, m2
    movd        [r1 + 1*mmsize], m2
    pshufd      m2, m2, 1
    movd        [r1 + 3*mmsize], m2

    pmulld      m2, m0, [r4 + 4*16]
    pmulld      m3, m1, [r4 + 4*16]
    pmulld      m4, m0, [r4 + 5*16]
    pmulld      m5, m1, [r4 + 5*16]
    phaddd      m2, m3
    phaddd      m4, m5
    phaddd      m2, m4                  ; m2 = [Row7 Row5]
    paddd       m2, m6
    psrad       m2, 9

    packssdw    m2, m2
    movd        [r1 + 5*mmsize], m2
    pshufd      m2, m2, 1
    movd        [r1 + 7*mmsize], m2

    add         r1, mmsize/4
    add         r0, 2*2*mmsize
%endrep

    dec         r2
    jnz        .pass2
    RET

%if ARCH_X86_64 == 1
%macro DCT8_PASS_1 4
    vpbroadcastq    m0,                 [r6 + %1]
    pmaddwd         m2,                 m%3, m0
    pmaddwd         m0,                 m%4
    phaddd          m2,                 m0
    paddd           m2,                 m5
    psrad           m2,                 DCT8_SHIFT1
    packssdw        m2,                 m2
    vpermq          m2,                 m2, 0x08
    mova            [r5 + %2],          xm2
%endmacro

%macro DCT8_PASS_2 2
    vbroadcasti128  m4,                 [r6 + %1]
    pmaddwd         m6,                 m0, m4
    pmaddwd         m7,                 m1, m4
    pmaddwd         m8,                 m2, m4
    pmaddwd         m9,                 m3, m4
    phaddd          m6,                 m7
    phaddd          m8,                 m9
    phaddd          m6,                 m8
    paddd           m6,                 m5
    psrad           m6,                 DCT8_SHIFT2

    vbroadcasti128  m4,                 [r6 + %2]
    pmaddwd         m10,                m0, m4
    pmaddwd         m7,                 m1, m4
    pmaddwd         m8,                 m2, m4
    pmaddwd         m9,                 m3, m4
    phaddd          m10,                m7
    phaddd          m8,                 m9
    phaddd          m10,                m8
    paddd           m10,                m5
    psrad           m10,                DCT8_SHIFT2

    packssdw        m6,                 m10
    vpermq          m10,                m6, 0xD8

%endmacro

INIT_YMM avx2
%if BIT_DEPTH == 12
cglobal dct8_12bit, 3, 7, 11, 0-8*16
%elif BIT_DEPTH == 10
cglobal dct8_10bit, 3, 7, 11, 0-8*16
%elif BIT_DEPTH == 8
cglobal dct8_8bit, 3, 7, 11, 0-8*16
%else
    %error Unsupported BIT_DEPTH!
%endif

vbroadcasti128      m5,                [pd_ %+ DCT8_ROUND1]
%define             DCT_SHIFT2         9

    add             r2d,               r2d
    lea             r3,                [r2 * 3]
    lea             r4,                [r0 + r2 * 4]
    mov             r5,                rsp
    lea             r6,                [tab_dct8]
    mova            m6,                [dct8_shuf]

    ;pass1
    mova            xm0,               [r0]
    vinserti128     m0,                m0, [r4], 1
    mova            xm1,               [r0 + r2]
    vinserti128     m1,                m1, [r4 + r2], 1
    mova            xm2,               [r0 + r2 * 2]
    vinserti128     m2,                m2, [r4 + r2 * 2], 1
    mova            xm3,               [r0 + r3]
    vinserti128     m3,                m3,  [r4 + r3], 1

    punpcklqdq      m4,                m0, m1
    punpckhqdq      m0,                m1
    punpcklqdq      m1,                m2, m3
    punpckhqdq      m2,                m3

    pshufb          m0,                m6
    pshufb          m2,                m6

    paddw           m3,                m4, m0
    paddw           m7,                m1, m2

    psubw           m4,                m0
    psubw           m1,                m2

    DCT8_PASS_1     0 * 16,             0 * 16, 3, 7
    DCT8_PASS_1     1 * 16,             2 * 16, 4, 1
    DCT8_PASS_1     2 * 16,             4 * 16, 3, 7
    DCT8_PASS_1     3 * 16,             6 * 16, 4, 1
    DCT8_PASS_1     4 * 16,             1 * 16, 3, 7
    DCT8_PASS_1     5 * 16,             3 * 16, 4, 1
    DCT8_PASS_1     6 * 16,             5 * 16, 3, 7
    DCT8_PASS_1     7 * 16,             7 * 16, 4, 1

    ;pass2
    vbroadcasti128  m5,                [pd_ %+ DCT8_ROUND2]

    mova            m0,                [r5]
    mova            m1,                [r5 + 32]
    mova            m2,                [r5 + 64]
    mova            m3,                [r5 + 96]

    DCT8_PASS_2     0 * 16, 1 * 16
    movu            [r1],              m10
    DCT8_PASS_2     2 * 16, 3 * 16
    movu            [r1 + 32],         m10
    DCT8_PASS_2     4 * 16, 5 * 16
    movu            [r1 + 64],         m10
    DCT8_PASS_2     6 * 16, 7 * 16
    movu            [r1 + 96],         m10
    RET


%macro DCT16_PASS_1_E 2
    vpbroadcastq    m7,                [r7 + %1]

    pmaddwd         m4,                m0, m7
    pmaddwd         m6,                m2, m7
    phaddd          m4,                m6

    paddd           m4,                m9
    psrad           m4,                DCT_SHIFT

    packssdw        m4,                m4
    vpermq          m4,                m4, 0x08

    mova            [r5 + %2],         xm4
%endmacro

%macro DCT16_PASS_1_O 2
    vbroadcasti128  m7,                [r7 + %1]

    pmaddwd         m10,               m0, m7
    pmaddwd         m11,               m2, m7
    phaddd          m10,               m11                 ; [d0 d0 d1 d1 d4 d4 d5 d5]

    pmaddwd         m11,               m4, m7
    pmaddwd         m12,               m6, m7
    phaddd          m11,               m12                 ; [d2 d2 d3 d3 d6 d6 d7 d7]

    phaddd          m10,               m11                 ; [d0 d1 d2 d3 d4 d5 d6 d7]

    paddd           m10,               m9
    psrad           m10,               DCT_SHIFT

    packssdw        m10,               m10                 ; [w0 w1 w2 w3 - - - - w4 w5 w6 w7 - - - -]
    vpermq          m10,               m10, 0x08

    mova            [r5 + %2],         xm10
%endmacro

%macro DCT16_PASS_2 2
    vbroadcasti128  m8,                [r7 + %1]
    vbroadcasti128  m13,               [r8 + %1]

    pmaddwd         m10,               m0, m8
    pmaddwd         m11,               m1, m13
    paddd           m10,               m11

    pmaddwd         m11,               m2, m8
    pmaddwd         m12,               m3, m13
    paddd           m11,               m12
    phaddd          m10,               m11

    pmaddwd         m11,               m4, m8
    pmaddwd         m12,               m5, m13
    paddd           m11,               m12

    pmaddwd         m12,               m6, m8
    pmaddwd         m13,               m7, m13
    paddd           m12,               m13
    phaddd          m11,               m12

    phaddd          m10,               m11
    paddd           m10,               m9
    psrad           m10,               DCT_SHIFT2


    vbroadcasti128  m8,                [r7 + %2]
    vbroadcasti128  m13,               [r8 + %2]

    pmaddwd         m14,               m0, m8
    pmaddwd         m11,               m1, m13
    paddd           m14,               m11

    pmaddwd         m11,               m2, m8
    pmaddwd         m12,               m3, m13
    paddd           m11,               m12
    phaddd          m14,               m11

    pmaddwd         m11,               m4, m8
    pmaddwd         m12,               m5, m13
    paddd           m11,               m12

    pmaddwd         m12,               m6, m8
    pmaddwd         m13,               m7, m13
    paddd           m12,               m13
    phaddd          m11,               m12

    phaddd          m14,               m11
    paddd           m14,               m9
    psrad           m14,               DCT_SHIFT2

    packssdw        m10,               m14
    vextracti128    xm14,              m10,       1
    movlhps         xm15,              xm10,      xm14
    movhlps         xm14,              xm10
%endmacro
INIT_YMM avx2
%if BIT_DEPTH == 12
cglobal dct16_12bit, 3, 9, 16, 0-16*mmsize
    %define         DCT_SHIFT          7
    vbroadcasti128  m9,                [pd_64]
%elif BIT_DEPTH == 10
cglobal dct16_10bit, 3, 9, 16, 0-16*mmsize
    %define         DCT_SHIFT          5
    vbroadcasti128  m9,                [pd_16]
%elif BIT_DEPTH == 8
cglobal dct16_8bit, 3, 9, 16, 0-16*mmsize
    %define         DCT_SHIFT          3
    vbroadcasti128  m9,                [pd_4]
%else
    %error Unsupported BIT_DEPTH!
%endif
%define             DCT_SHIFT2         10

    add             r2d,               r2d

    mova            m13,               [dct16_shuf1]
    mova            m14,               [dct16_shuf2]
    lea             r7,                [tab_dct16_1 + 8 * 16]
    lea             r8,                [tab_dct16_2 + 8 * 16]
    lea             r3,                [r2 * 3]
    mov             r5,                rsp
    mov             r4d,               2                   ; Each iteration process 8 rows, so 16/8 iterations

.pass1:
    lea             r6,                [r0 + r2 * 4]

    movu            m2,                [r0]
    movu            m1,                [r6]
    vperm2i128      m0,                m2, m1, 0x20        ; [row0lo  row4lo]
    vperm2i128      m1,                m2, m1, 0x31        ; [row0hi  row4hi]

    movu            m4,                [r0 + r2]
    movu            m3,                [r6 + r2]
    vperm2i128      m2,                m4, m3, 0x20        ; [row1lo  row5lo]
    vperm2i128      m3,                m4, m3, 0x31        ; [row1hi  row5hi]

    movu            m6,                [r0 + r2 * 2]
    movu            m5,                [r6 + r2 * 2]
    vperm2i128      m4,                m6, m5, 0x20        ; [row2lo  row6lo]
    vperm2i128      m5,                m6, m5, 0x31        ; [row2hi  row6hi]

    movu            m8,                [r0 + r3]
    movu            m7,                [r6 + r3]
    vperm2i128      m6,                m8, m7, 0x20        ; [row3lo  row7lo]
    vperm2i128      m7,                m8, m7, 0x31        ; [row3hi  row7hi]

    pshufb          m1,                m13
    pshufb          m3,                m13
    pshufb          m5,                m13
    pshufb          m7,                m13

    paddw           m8,                m0, m1              ;E
    psubw           m0,                m1                  ;O

    paddw           m1,                m2, m3              ;E
    psubw           m2,                m3                  ;O

    paddw           m3,                m4, m5              ;E
    psubw           m4,                m5                  ;O

    paddw           m5,                m6, m7              ;E
    psubw           m6,                m7                  ;O

    DCT16_PASS_1_O  -7 * 16,           1 * 32
    DCT16_PASS_1_O  -5 * 16,           3 * 32
    DCT16_PASS_1_O  -3 * 16,           1 * 32 + 16
    DCT16_PASS_1_O  -1 * 16,           3 * 32 + 16
    DCT16_PASS_1_O  1 * 16,            5 * 32
    DCT16_PASS_1_O  3 * 16,            7 * 32
    DCT16_PASS_1_O  5 * 16,            5 * 32 + 16
    DCT16_PASS_1_O  7 * 16,            7 * 32 + 16

    pshufb          m8,                m14
    pshufb          m1,                m14
    phaddw          m0,                m8, m1

    pshufb          m3,                m14
    pshufb          m5,                m14
    phaddw          m2,                m3, m5

    DCT16_PASS_1_E  -8 * 16,           0 * 32
    DCT16_PASS_1_E  -4 * 16,           0 * 32 + 16
    DCT16_PASS_1_E  0 * 16,            4 * 32
    DCT16_PASS_1_E  4 * 16,            4 * 32 + 16

    phsubw          m0,                m8, m1
    phsubw          m2,                m3, m5

    DCT16_PASS_1_E  -6 * 16,           2 * 32
    DCT16_PASS_1_E  -2 * 16,           2 * 32 + 16
    DCT16_PASS_1_E  2 * 16,            6 * 32
    DCT16_PASS_1_E  6 * 16,            6 * 32 + 16

    lea             r0,                [r0 + 8 * r2]
    add             r5,                256

    dec             r4d
    jnz             .pass1

    mov             r5,                rsp
    mov             r4d,               2
    mov             r2d,               32
    lea             r3,                [r2 * 3]
    vbroadcasti128  m9,                [pd_512]

.pass2:
    mova            m0,                [r5 + 0 * 32]        ; [row0lo  row4lo]
    mova            m1,                [r5 + 8 * 32]        ; [row0hi  row4hi]

    mova            m2,                [r5 + 1 * 32]        ; [row1lo  row5lo]
    mova            m3,                [r5 + 9 * 32]        ; [row1hi  row5hi]

    mova            m4,                [r5 + 2 * 32]        ; [row2lo  row6lo]
    mova            m5,                [r5 + 10 * 32]       ; [row2hi  row6hi]

    mova            m6,                [r5 + 3 * 32]        ; [row3lo  row7lo]
    mova            m7,                [r5 + 11 * 32]       ; [row3hi  row7hi]

    DCT16_PASS_2    -8 * 16, -7 * 16
    movu            [r1],              xm15
    movu            [r1 + r2],         xm14

    DCT16_PASS_2    -6 * 16, -5 * 16
    movu            [r1 + r2 * 2],     xm15
    movu            [r1 + r3],         xm14

    lea             r6,                [r1 + r2 * 4]
    DCT16_PASS_2    -4 * 16, -3 * 16
    movu            [r6],              xm15
    movu            [r6 + r2],         xm14

    DCT16_PASS_2    -2 * 16, -1 * 16
    movu            [r6 + r2 * 2],     xm15
    movu            [r6 + r3],         xm14

    lea             r6,                [r6 + r2 * 4]
    DCT16_PASS_2    0 * 16, 1 * 16
    movu            [r6],              xm15
    movu            [r6 + r2],         xm14

    DCT16_PASS_2    2 * 16, 3 * 16
    movu            [r6 + r2 * 2],     xm15
    movu            [r6 + r3],         xm14

    lea             r6,                [r6 + r2 * 4]
    DCT16_PASS_2    4 * 16, 5 * 16
    movu            [r6],              xm15
    movu            [r6 + r2],         xm14

    DCT16_PASS_2    6 * 16, 7 * 16
    movu            [r6 + r2 * 2],     xm15
    movu            [r6 + r3],         xm14

    add             r1,                16
    add             r5,                128

    dec             r4d
    jnz             .pass2
    RET

%macro DCT32_PASS_1 4
    vbroadcasti128  m8,                [r7 + %1]
    pmaddwd         m11,               m%3, m8
    pmaddwd         m12,               m%4, m8
    phaddd          m11,               m12

    vbroadcasti128  m8,                [r7 + %1 + 32]
    vbroadcasti128  m10,               [r7 + %1 + 48]
    pmaddwd         m12,               m5, m8
    pmaddwd         m13,               m6, m10
    phaddd          m12,               m13

    pmaddwd         m13,               m4, m8
    pmaddwd         m14,               m7, m10
    phaddd          m13,               m14

    phaddd          m12,               m13

    phaddd          m11,               m12
    paddd           m11,               m9
    psrad           m11,               DCT_SHIFT

    vpermq          m11,               m11, 0xD8
    packssdw        m11,               m11
    movq            [r5 + %2],         xm11
    vextracti128    xm10,              m11, 1
    movq            [r5 + %2 + 64],    xm10
%endmacro

%macro DCT32_PASS_2 1
    mova            m8,                [r7 + %1]
    mova            m10,               [r8 + %1]
    pmaddwd         m11,               m0, m8
    pmaddwd         m12,               m1, m10
    paddd           m11,               m12

    pmaddwd         m12,               m2, m8
    pmaddwd         m13,               m3, m10
    paddd           m12,               m13

    phaddd          m11,               m12

    pmaddwd         m12,               m4, m8
    pmaddwd         m13,               m5, m10
    paddd           m12,               m13

    pmaddwd         m13,               m6, m8
    pmaddwd         m14,               m7, m10
    paddd           m13,               m14

    phaddd          m12,               m13

    phaddd          m11,               m12
    vextracti128    xm10,              m11, 1
    paddd           xm11,              xm10

    paddd           xm11,               xm9
    psrad           xm11,               DCT_SHIFT2
    packssdw        xm11,               xm11

%endmacro

INIT_YMM avx2
%if BIT_DEPTH == 12
cglobal dct32_12bit, 3, 9, 16, 0-64*mmsize
    %define         DCT_SHIFT          8
    vpbroadcastq    m9,                [pd_128]
%elif BIT_DEPTH == 10
cglobal dct32_10bit, 3, 9, 16, 0-64*mmsize
    %define         DCT_SHIFT          6
    vpbroadcastq    m9,                [pd_32]
%elif BIT_DEPTH == 8
cglobal dct32_8bit, 3, 9, 16, 0-64*mmsize
    %define         DCT_SHIFT          4
    vpbroadcastq    m9,                [pd_8]
%else
    %error Unsupported BIT_DEPTH!
%endif
%define             DCT_SHIFT2         11

    add             r2d,               r2d

    lea             r7,                [tab_dct32_1]
    lea             r8,                [tab_dct32_2]
    lea             r3,                [r2 * 3]
    mov             r5,                rsp
    mov             r4d,               8
    mova            m15,               [dct16_shuf1]

.pass1:
    movu            m2,                [r0]
    movu            m1,                [r0 + 32]
    pshufb          m1,                m15
    vpermq          m1,                m1, 0x4E
    psubw           m7,                m2, m1
    paddw           m2,                m1

    movu            m1,                [r0 + r2 * 2]
    movu            m0,                [r0 + r2 * 2 + 32]
    pshufb          m0,                m15
    vpermq          m0,                m0, 0x4E
    psubw           m8,                m1, m0
    paddw           m1,                m0
    vperm2i128      m0,                m2, m1, 0x20        ; [row0lo  row2lo] for E
    vperm2i128      m3,                m2, m1, 0x31        ; [row0hi  row2hi] for E
    pshufb          m3,                m15
    psubw           m1,                m0, m3
    paddw           m0,                m3

    vperm2i128      m5,                m7, m8, 0x20        ; [row0lo  row2lo] for O
    vperm2i128      m6,                m7, m8, 0x31        ; [row0hi  row2hi] for O


    movu            m4,                [r0 + r2]
    movu            m2,                [r0 + r2 + 32]
    pshufb          m2,                m15
    vpermq          m2,                m2, 0x4E
    psubw           m10,               m4, m2
    paddw           m4,                m2

    movu            m3,                [r0 + r3]
    movu            m2,                [r0 + r3 + 32]
    pshufb          m2,                m15
    vpermq          m2,                m2, 0x4E
    psubw           m11,               m3, m2
    paddw           m3,                m2
    vperm2i128      m2,                m4, m3, 0x20        ; [row1lo  row3lo] for E
    vperm2i128      m8,                m4, m3, 0x31        ; [row1hi  row3hi] for E
    pshufb          m8,                m15
    psubw           m3,                m2, m8
    paddw           m2,                m8

    vperm2i128      m4,                m10, m11, 0x20      ; [row1lo  row3lo] for O
    vperm2i128      m7,                m10, m11, 0x31      ; [row1hi  row3hi] for O


    DCT32_PASS_1    0 * 32,            0 * 64, 0, 2
    DCT32_PASS_1    2 * 32,            2 * 64, 1, 3
    DCT32_PASS_1    4 * 32,            4 * 64, 0, 2
    DCT32_PASS_1    6 * 32,            6 * 64, 1, 3
    DCT32_PASS_1    8 * 32,            8 * 64, 0, 2
    DCT32_PASS_1    10 * 32,           10 * 64, 1, 3
    DCT32_PASS_1    12 * 32,           12 * 64, 0, 2
    DCT32_PASS_1    14 * 32,           14 * 64, 1, 3
    DCT32_PASS_1    16 * 32,           16 * 64, 0, 2
    DCT32_PASS_1    18 * 32,           18 * 64, 1, 3
    DCT32_PASS_1    20 * 32,           20 * 64, 0, 2
    DCT32_PASS_1    22 * 32,           22 * 64, 1, 3
    DCT32_PASS_1    24 * 32,           24 * 64, 0, 2
    DCT32_PASS_1    26 * 32,           26 * 64, 1, 3
    DCT32_PASS_1    28 * 32,           28 * 64, 0, 2
    DCT32_PASS_1    30 * 32,           30 * 64, 1, 3

    add             r5,                8
    lea             r0,                [r0 + r2 * 4]

    dec             r4d
    jnz             .pass1

    mov             r2d,               64
    lea             r3,                [r2 * 3]
    mov             r5,                rsp
    mov             r4d,               8
    vpbroadcastq    m9,                [pd_1024]

.pass2:
    mova            m0,                [r5 + 0 * 64]
    mova            m1,                [r5 + 0 * 64 + 32]

    mova            m2,                [r5 + 1 * 64]
    mova            m3,                [r5 + 1 * 64 + 32]

    mova            m4,                [r5 + 2 * 64]
    mova            m5,                [r5 + 2 * 64 + 32]

    mova            m6,                [r5 + 3 * 64]
    mova            m7,                [r5 + 3 * 64 + 32]

    DCT32_PASS_2    0 * 32
    movq            [r1],              xm11
    DCT32_PASS_2    1 * 32
    movq            [r1 + r2],         xm11
    DCT32_PASS_2    2 * 32
    movq            [r1 + r2 * 2],     xm11
    DCT32_PASS_2    3 * 32
    movq            [r1 + r3],         xm11

    lea             r6,                [r1 + r2 * 4]
    DCT32_PASS_2    4 * 32
    movq            [r6],              xm11
    DCT32_PASS_2    5 * 32
    movq            [r6 + r2],         xm11
    DCT32_PASS_2    6 * 32
    movq            [r6 + r2 * 2],     xm11
    DCT32_PASS_2    7 * 32
    movq            [r6 + r3],         xm11

    lea             r6,                [r6 + r2 * 4]
    DCT32_PASS_2    8 * 32
    movq            [r6],              xm11
    DCT32_PASS_2    9 * 32
    movq            [r6 + r2],         xm11
    DCT32_PASS_2    10 * 32
    movq            [r6 + r2 * 2],     xm11
    DCT32_PASS_2    11 * 32
    movq            [r6 + r3],         xm11

    lea             r6,                [r6 + r2 * 4]
    DCT32_PASS_2    12 * 32
    movq            [r6],              xm11
    DCT32_PASS_2    13 * 32
    movq            [r6 + r2],         xm11
    DCT32_PASS_2    14 * 32
    movq            [r6 + r2 * 2],     xm11
    DCT32_PASS_2    15 * 32
    movq            [r6 + r3],         xm11

    lea             r6,                [r6 + r2 * 4]
    DCT32_PASS_2    16 * 32
    movq            [r6],              xm11
    DCT32_PASS_2    17 * 32
    movq            [r6 + r2],         xm11
    DCT32_PASS_2    18 * 32
    movq            [r6 + r2 * 2],     xm11
    DCT32_PASS_2    19 * 32
    movq            [r6 + r3],         xm11

    lea             r6,                [r6 + r2 * 4]
    DCT32_PASS_2    20 * 32
    movq            [r6],              xm11
    DCT32_PASS_2    21 * 32
    movq            [r6 + r2],         xm11
    DCT32_PASS_2    22 * 32
    movq            [r6 + r2 * 2],     xm11
    DCT32_PASS_2    23 * 32
    movq            [r6 + r3],         xm11

    lea             r6,                [r6 + r2 * 4]
    DCT32_PASS_2    24 * 32
    movq            [r6],              xm11
    DCT32_PASS_2    25 * 32
    movq            [r6 + r2],         xm11
    DCT32_PASS_2    26 * 32
    movq            [r6 + r2 * 2],     xm11
    DCT32_PASS_2    27 * 32
    movq            [r6 + r3],         xm11

    lea             r6,                [r6 + r2 * 4]
    DCT32_PASS_2    28 * 32
    movq            [r6],              xm11
    DCT32_PASS_2    29 * 32
    movq            [r6 + r2],         xm11
    DCT32_PASS_2    30 * 32
    movq            [r6 + r2 * 2],     xm11
    DCT32_PASS_2    31 * 32
    movq            [r6 + r3],         xm11

    add             r5,                256
    add             r1,                8

    dec             r4d
    jnz             .pass2
    RET

%endif
