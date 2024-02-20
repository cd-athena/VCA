/*****************************************************************************
 * Copyright (C) 2024 Christian Doppler Laboratory ATHENA
 *
 * Authors: Mandar Gurav <mandar@multicorewareinc.com>
 *          Deepthi Devaki Akkoorath <deepthidevaki@multicorewareinc.com>
 *          Mahesh Pittala <mahesh@multicorewareinc.com>
 *          Rajesh Paulraj <rajesh@multicorewareinc.com>
 *          Min Chen <min.chen@multicorewareinc.com>
 *          Praveen Kumar Tiwari <praveen@multicorewareinc.com>
 *          Nabajit Deka <nabajit@multicorewareinc.com>
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

#include <cstdint>

namespace vca {

typedef void (*dct_t)(const int16_t *src, int16_t *dst, intptr_t srcStride);

void dct8_c(const int16_t *src, int16_t *dst, intptr_t srcStride, const unsigned bitDepth);
void dct16_c(const int16_t *src, int16_t *dst, intptr_t srcStride, const unsigned bitDepth);
void dct32_c(const int16_t *src, int16_t *dst, intptr_t srcStride, const unsigned bitDepth);

} // namespace vca