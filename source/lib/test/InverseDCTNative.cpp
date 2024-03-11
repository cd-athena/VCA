/* Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Christian Feldmann <christian.feldmann@bitmovin.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

#include "InverseDCTNative.h"

#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>

#if defined(__GNUC__)
#define ALIGN_VAR_32(T, var) T var __attribute__((aligned(32)))
#elif defined(_MSC_VER)
#define ALIGN_VAR_32(T, var) __declspec(align(32)) T var
#endif

namespace {

template<typename T>
inline T x265_clip3(T minVal, T maxVal, T a)
{
    return std::min(std::max(minVal, a), maxVal);
}

const int16_t g_t8[8][8] = {{64, 64, 64, 64, 64, 64, 64, 64},
                            {89, 75, 50, 18, -18, -50, -75, -89},
                            {83, 36, -36, -83, -83, -36, 36, 83},
                            {75, -18, -89, -50, 50, 89, 18, -75},
                            {64, -64, -64, 64, 64, -64, -64, 64},
                            {50, -89, 18, 75, -75, -18, 89, -50},
                            {36, -83, 83, -36, -36, 83, -83, 36},
                            {18, -50, 75, -89, 89, -75, 50, -18}};

const int16_t g_t16[16][16]
    = {{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
       {90, 87, 80, 70, 57, 43, 25, 9, -9, -25, -43, -57, -70, -80, -87, -90},
       {89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89},
       {87, 57, 9, -43, -80, -90, -70, -25, 25, 70, 90, 80, 43, -9, -57, -87},
       {83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83},
       {80, 9, -70, -87, -25, 57, 90, 43, -43, -90, -57, 25, 87, 70, -9, -80},
       {75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75},
       {70, -43, -87, 9, 90, 25, -80, -57, 57, 80, -25, -90, -9, 87, 43, -70},
       {64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64},
       {57, -80, -25, 90, -9, -87, 43, 70, -70, -43, 87, 9, -90, 25, 80, -57},
       {50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50},
       {43, -90, 57, 25, -87, 70, 9, -80, 80, -9, -70, 87, -25, -57, 90, -43},
       {36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36},
       {25, -70, 90, -80, 43, 9, -57, 87, -87, 57, -9, -43, 80, -90, 70, -25},
       {18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18},
       {9, -25, 43, -57, 70, -80, 87, -90, 90, -87, 80, -70, 57, -43, 25, -9}};

const int16_t g_t32[32][32]
    = {{64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64},
       {90, 90,  88,  85,  82,  78,  73,  67,  61,  54,  46,  38,  31,  22,  13,  4,
        -4, -13, -22, -31, -38, -46, -54, -61, -67, -73, -78, -82, -85, -88, -90, -90},
       {90,  87,  80,  70,  57,  43,  25,  9,  -9, -25, -43, -57, -70, -80, -87, -90,
        -90, -87, -80, -70, -57, -43, -25, -9, 9,  25,  43,  57,  70,  80,  87,  90},
       {90, 82, 67, 46, 22, -4, -31, -54, -73, -85, -90, -88, -78, -61, -38, -13,
        13, 38, 61, 78, 88, 90, 85,  73,  54,  31,  4,   -22, -46, -67, -82, -90},
       {89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89,
        89, 75, 50, 18, -18, -50, -75, -89, -89, -75, -50, -18, 18, 50, 75, 89},
       {88,  67,  31,  -13, -54, -82, -90, -78, -46, -4, 38, 73, 90, 85,  61,  22,
        -22, -61, -85, -90, -73, -38, 4,   46,  78,  90, 82, 54, 13, -31, -67, -88},
       {87,  57,  9,  -43, -80, -90, -70, -25, 25,  70,  90,  80,  43,  -9, -57, -87,
        -87, -57, -9, 43,  80,  90,  70,  25,  -25, -70, -90, -80, -43, 9,  57,  87},
       {85, 46, -13, -67, -90, -73, -22, 38,  82,  88, 54, -4, -61, -90, -78, -31,
        31, 78, 90,  61,  4,   -54, -88, -82, -38, 22, 73, 90, 67,  13,  -46, -85},
       {83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83,
        83, 36, -36, -83, -83, -36, 36, 83, 83, 36, -36, -83, -83, -36, 36, 83},
       {82,  22,  -54, -90, -61, 13, 78, 85,  31,  -46, -90, -67, 4,  73, 88,  38,
        -38, -88, -73, -4,  67,  90, 46, -31, -85, -78, -13, 61,  90, 54, -22, -82},
       {80,  9,  -70, -87, -25, 57,  90,  43,  -43, -90, -57, 25,  87,  70,  -9, -80,
        -80, -9, 70,  87,  25,  -57, -90, -43, 43,  90,  57,  -25, -87, -70, 9,  80},
       {78, -4, -82, -73, 13,  85,  67, -22, -88, -61, 31,  90,  54, -38, -90, -46,
        46, 90, 38,  -54, -90, -31, 61, 88,  22,  -67, -85, -13, 73, 82,  4,   -78},
       {75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75,
        75, -18, -89, -50, 50, 89, 18, -75, -75, 18, 89, 50, -50, -89, -18, 75},
       {73,  -31, -90, -22, 78, 67,  -38, -90, -13, 82, 61,  -46, -88, -4, 85, 54,
        -54, -85, 4,   88,  46, -61, -82, 13,  90,  38, -67, -78, 22,  90, 31, -73},
       {70,  -43, -87, 9,  90,  25,  -80, -57, 57,  80,  -25, -90, -9, 87,  43,  -70,
        -70, 43,  87,  -9, -90, -25, 80,  57,  -57, -80, 25,  90,  9,  -87, -43, 70},
       {67, -54, -78, 38,  85, -22, -90, 4,   90, 13, -88, -31, 82,  46, -73, -61,
        61, 73,  -46, -82, 31, 88,  -13, -90, -4, 90, 22,  -85, -38, 78, 54,  -67},
       {64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64,
        64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64, 64, -64, -64, 64},
       {61,  -73, -46, 82, 31,  -88, -13, 90, -4,  -90, 22, 85,  -38, -78, 54, 67,
        -67, -54, 78,  38, -85, -22, 90,  4,  -90, 13,  88, -31, -82, 46,  73, -61},
       {57,  -80, -25, 90,  -9, -87, 43,  70,  -70, -43, 87,  9,  -90, 25,  80,  -57,
        -57, 80,  25,  -90, 9,  87,  -43, -70, 70,  43,  -87, -9, 90,  -25, -80, 57},
       {54, -85, -4,  88, -46, -61, 82,  13, -90, 38,  67, -78, -22, 90, -31, -73,
        73, 31,  -90, 22, 78,  -67, -38, 90, -13, -82, 61, 46,  -88, 4,  85,  -54},
       {50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50,
        50, -89, 18, 75, -75, -18, 89, -50, -50, 89, -18, -75, 75, 18, -89, 50},
       {46,  -90, 38, 54,  -90, 31, 61,  -88, 22, 67,  -85, 13, 73,  -82, 4,  78,
        -78, -4,  82, -73, -13, 85, -67, -22, 88, -61, -31, 90, -54, -38, 90, -46},
       {43,  -90, 57,  25,  -87, 70,  9,  -80, 80,  -9, -70, 87,  -25, -57, 90,  -43,
        -43, 90,  -57, -25, 87,  -70, -9, 80,  -80, 9,  70,  -87, 25,  57,  -90, 43},
       {38, -88, 73,  -4, -67, 90,  -46, -31, 85, -78, 13,  61, -90, 54,  22, -82,
        82, -22, -54, 90, -61, -13, 78,  -85, 31, 46,  -90, 67, 4,   -73, 88, -38},
       {36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36,
        36, -83, 83, -36, -36, 83, -83, 36, 36, -83, 83, -36, -36, 83, -83, 36},
       {31,  -78, 90, -61, 4,  54,  -88, 82, -38, -22, 73,  -90, 67, -13, -46, 85,
        -85, 46,  13, -67, 90, -73, 22,  38, -82, 88,  -54, -4,  61, -90, 78,  -31},
       {25,  -70, 90,  -80, 43,  9,  -57, 87,  -87, 57,  -9, -43, 80,  -90, 70,  -25,
        -25, 70,  -90, 80,  -43, -9, 57,  -87, 87,  -57, 9,  43,  -80, 90,  -70, 25},
       {22, -61, 85, -90, 73,  -38, -4,  46, -78, 90, -82, 54,  -13, -31, 67, -88,
        88, -67, 31, 13,  -54, 82,  -90, 78, -46, 4,  38,  -73, 90,  -85, 61, -22},
       {18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18,
        18, -50, 75, -89, 89, -75, 50, -18, -18, 50, -75, 89, -89, 75, -50, 18},
       {13,  -38, 61,  -78, 88,  -90, 85, -73, 54, -31, 4,  22,  -46, 67,  -82, 90,
        -90, 82,  -67, 46,  -22, -4,  31, -54, 73, -85, 90, -88, 78,  -61, 38,  -13},
       {9,  -25, 43,  -57, 70,  -80, 87,  -90, 90,  -87, 80,  -70, 57,  -43, 25,  -9,
        -9, 25,  -43, 57,  -70, 80,  -87, 90,  -90, 87,  -80, 70,  -57, 43,  -25, 9},
       {4,  -13, 22, -31, 38, -46, 54, -61, 67, -73, 78, -82, 85, -88, 90, -90,
        90, -90, 88, -85, 82, -78, 73, -67, 61, -54, 46, -38, 31, -22, 13, -4}};

void partialButterflyInverse8(const int16_t *src, int16_t *dst, int shift, int line)
{
    int j, k;
    int E[4], O[4];
    int EE[2], EO[2];
    int add = 1 << (shift - 1);

    for (j = 0; j < line; j++)
    {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 4; k++)
        {
            O[k] = g_t8[1][k] * src[line] + g_t8[3][k] * src[3 * line] + g_t8[5][k] * src[5 * line]
                   + g_t8[7][k] * src[7 * line];
        }

        EO[0] = g_t8[2][0] * src[2 * line] + g_t8[6][0] * src[6 * line];
        EO[1] = g_t8[2][1] * src[2 * line] + g_t8[6][1] * src[6 * line];
        EE[0] = g_t8[0][0] * src[0] + g_t8[4][0] * src[4 * line];
        EE[1] = g_t8[0][1] * src[0] + g_t8[4][1] * src[4 * line];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial
         * domain vector */
        E[0] = EE[0] + EO[0];
        E[3] = EE[0] - EO[0];
        E[1] = EE[1] + EO[1];
        E[2] = EE[1] - EO[1];
        for (k = 0; k < 4; k++)
        {
            dst[k]     = (int16_t) x265_clip3(-32768, 32767, (E[k] + O[k] + add) >> shift);
            dst[k + 4] = (int16_t) x265_clip3(-32768, 32767, (E[3 - k] - O[3 - k] + add) >> shift);
        }

        src++;
        dst += 8;
    }
}

void partialButterflyInverse16(const int16_t *src, int16_t *dst, int shift, int line)
{
    int j, k;
    int E[8], O[8];
    int EE[4], EO[4];
    int EEE[2], EEO[2];
    int add = 1 << (shift - 1);

    for (j = 0; j < line; j++)
    {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 8; k++)
        {
            O[k] = g_t16[1][k] * src[line] + g_t16[3][k] * src[3 * line]
                   + g_t16[5][k] * src[5 * line] + g_t16[7][k] * src[7 * line]
                   + g_t16[9][k] * src[9 * line] + g_t16[11][k] * src[11 * line]
                   + g_t16[13][k] * src[13 * line] + g_t16[15][k] * src[15 * line];
        }

        for (k = 0; k < 4; k++)
        {
            EO[k] = g_t16[2][k] * src[2 * line] + g_t16[6][k] * src[6 * line]
                    + g_t16[10][k] * src[10 * line] + g_t16[14][k] * src[14 * line];
        }

        EEO[0] = g_t16[4][0] * src[4 * line] + g_t16[12][0] * src[12 * line];
        EEE[0] = g_t16[0][0] * src[0] + g_t16[8][0] * src[8 * line];
        EEO[1] = g_t16[4][1] * src[4 * line] + g_t16[12][1] * src[12 * line];
        EEE[1] = g_t16[0][1] * src[0] + g_t16[8][1] * src[8 * line];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial
         * domain vector */
        for (k = 0; k < 2; k++)
        {
            EE[k]     = EEE[k] + EEO[k];
            EE[k + 2] = EEE[1 - k] - EEO[1 - k];
        }

        for (k = 0; k < 4; k++)
        {
            E[k]     = EE[k] + EO[k];
            E[k + 4] = EE[3 - k] - EO[3 - k];
        }

        for (k = 0; k < 8; k++)
        {
            dst[k]     = (int16_t) x265_clip3(-32768, 32767, (E[k] + O[k] + add) >> shift);
            dst[k + 8] = (int16_t) x265_clip3(-32768, 32767, (E[7 - k] - O[7 - k] + add) >> shift);
        }

        src++;
        dst += 16;
    }
}

void partialButterflyInverse32(const int16_t *src, int16_t *dst, int shift, int line)
{
    int j, k;
    int E[16], O[16];
    int EE[8], EO[8];
    int EEE[4], EEO[4];
    int EEEE[2], EEEO[2];
    int add = 1 << (shift - 1);

    for (j = 0; j < line; j++)
    {
        /* Utilizing symmetry properties to the maximum to minimize the number of multiplications */
        for (k = 0; k < 16; k++)
        {
            O[k] = g_t32[1][k] * src[line] + g_t32[3][k] * src[3 * line]
                   + g_t32[5][k] * src[5 * line] + g_t32[7][k] * src[7 * line]
                   + g_t32[9][k] * src[9 * line] + g_t32[11][k] * src[11 * line]
                   + g_t32[13][k] * src[13 * line] + g_t32[15][k] * src[15 * line]
                   + g_t32[17][k] * src[17 * line] + g_t32[19][k] * src[19 * line]
                   + g_t32[21][k] * src[21 * line] + g_t32[23][k] * src[23 * line]
                   + g_t32[25][k] * src[25 * line] + g_t32[27][k] * src[27 * line]
                   + g_t32[29][k] * src[29 * line] + g_t32[31][k] * src[31 * line];
        }

        for (k = 0; k < 8; k++)
        {
            EO[k] = g_t32[2][k] * src[2 * line] + g_t32[6][k] * src[6 * line]
                    + g_t32[10][k] * src[10 * line] + g_t32[14][k] * src[14 * line]
                    + g_t32[18][k] * src[18 * line] + g_t32[22][k] * src[22 * line]
                    + g_t32[26][k] * src[26 * line] + g_t32[30][k] * src[30 * line];
        }

        for (k = 0; k < 4; k++)
        {
            EEO[k] = g_t32[4][k] * src[4 * line] + g_t32[12][k] * src[12 * line]
                     + g_t32[20][k] * src[20 * line] + g_t32[28][k] * src[28 * line];
        }

        EEEO[0] = g_t32[8][0] * src[8 * line] + g_t32[24][0] * src[24 * line];
        EEEO[1] = g_t32[8][1] * src[8 * line] + g_t32[24][1] * src[24 * line];
        EEEE[0] = g_t32[0][0] * src[0] + g_t32[16][0] * src[16 * line];
        EEEE[1] = g_t32[0][1] * src[0] + g_t32[16][1] * src[16 * line];

        /* Combining even and odd terms at each hierarchy levels to calculate the final spatial
         * domain vector */
        EEE[0] = EEEE[0] + EEEO[0];
        EEE[3] = EEEE[0] - EEEO[0];
        EEE[1] = EEEE[1] + EEEO[1];
        EEE[2] = EEEE[1] - EEEO[1];
        for (k = 0; k < 4; k++)
        {
            EE[k]     = EEE[k] + EEO[k];
            EE[k + 4] = EEE[3 - k] - EEO[3 - k];
        }

        for (k = 0; k < 8; k++)
        {
            E[k]     = EE[k] + EO[k];
            E[k + 8] = EE[7 - k] - EO[7 - k];
        }

        for (k = 0; k < 16; k++)
        {
            dst[k]      = (int16_t) x265_clip3(-32768, 32767, (E[k] + O[k] + add) >> shift);
            dst[k + 16] = (int16_t) x265_clip3(-32768,
                                               32767,
                                               (E[15 - k] - O[15 - k] + add) >> shift);
        }

        src++;
        dst += 32;
    }
}

void idct8_c(const int16_t *src, int16_t *dst, intptr_t dstStride, const unsigned bitDepth)
{
    const int shift_1st = 7;
    const int shift_2nd = 12 - (bitDepth - 8);

    ALIGN_VAR_32(int16_t, coef[8 * 8]);
    ALIGN_VAR_32(int16_t, block[8 * 8]);

    partialButterflyInverse8(src, coef, shift_1st, 8);
    partialButterflyInverse8(coef, block, shift_2nd, 8);

    for (int i = 0; i < 8; i++)
    {
        std::memcpy(&dst[i * dstStride], &block[i * 8], 8 * sizeof(int16_t));
    }
}

void idct16_c(const int16_t *src, int16_t *dst, intptr_t dstStride, const unsigned bitDepth)
{
    const int shift_1st = 7;
    const int shift_2nd = 12 - (bitDepth - 8);

    ALIGN_VAR_32(int16_t, coef[16 * 16]);
    ALIGN_VAR_32(int16_t, block[16 * 16]);

    partialButterflyInverse16(src, coef, shift_1st, 16);
    partialButterflyInverse16(coef, block, shift_2nd, 16);

    for (int i = 0; i < 16; i++)
    {
        std::memcpy(&dst[i * dstStride], &block[i * 16], 16 * sizeof(int16_t));
    }
}

void idct32_c(const int16_t *src, int16_t *dst, intptr_t dstStride, const unsigned bitDepth)
{
    const int shift_1st = 7;
    const int shift_2nd = 12 - (bitDepth - 8);

    ALIGN_VAR_32(int16_t, coef[32 * 32]);
    ALIGN_VAR_32(int16_t, block[32 * 32]);

    partialButterflyInverse32(src, coef, shift_1st, 32);
    partialButterflyInverse32(coef, block, shift_2nd, 32);

    for (int i = 0; i < 32; i++)
    {
        std::memcpy(&dst[i * dstStride], &block[i * 32], 32 * sizeof(int16_t));
    }
}

} // namespace

namespace test {

void performIDCT(const unsigned blockSize,
                 const unsigned bitDepth,
                 int16_t *coeffBuffer,
                 int16_t *pixelBuffer)
{
    if (bitDepth != 8 && bitDepth != 10 && bitDepth != 12)
        throw std::invalid_argument("Invalid bit depth " + std::to_string(bitDepth));

    switch (blockSize)
    {
        case 32:
            idct32_c(coeffBuffer, pixelBuffer, 32, bitDepth);
            break;
        case 16:
            idct16_c(coeffBuffer, pixelBuffer, 16, bitDepth);
            break;
        case 8:
            idct8_c(coeffBuffer, pixelBuffer, 8, bitDepth);
            break;
        default:
            throw std::invalid_argument("Invalid block size " + std::to_string(blockSize));
    }
}

} // namespace test