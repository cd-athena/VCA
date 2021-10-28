/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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

#ifndef VCA_CONSTANTS_H
#define VCA_CONSTANTS_H

#include "common.h"

namespace VCA_NS {

extern const uint8_t g_log2Size[MAX_CU_SIZE + 1]; // from size to log2(size)
extern const int16_t g_t4[4][4];
extern const int16_t g_t8[8][8];
extern const int16_t g_t16[16][16];
extern const int16_t g_t32[32][32];

}

#endif
