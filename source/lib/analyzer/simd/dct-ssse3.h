/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
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

#pragma once

#include <stdint.h>

void vca_dct16_8bit_ssse3(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct16_10bit_ssse3(const int16_t *src, int16_t *dst, intptr_t srcStride);
void vca_dct16_12bit_ssse3(const int16_t *src, int16_t *dst, intptr_t srcStride);

void vca_dct32_8bit_ssse3(const int16_t *src, int16_t *dst, intptr_t stride);
void vca_dct32_10bit_ssse3(const int16_t *src, int16_t *dst, intptr_t stride);
void vca_dct32_12bit_ssse3(const int16_t *src, int16_t *dst, intptr_t stride);
