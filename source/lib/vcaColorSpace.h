/*****************************************************************************
 * Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Christian Feldmann <christian.feldmann@bitmovin.com>
 *          Vignesh V Menon <vignesh.menon@aau.at>       
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

#include <map>

// TODO: This should go into a namespace

enum class vca_colorSpace
{
    YUV400,
    YUV420,
    YUV422,
    YUV444
};

struct vca_cli_csp
{
    int planes;
    int width[3];
    int height[3];
};

static const std::map<vca_colorSpace, vca_cli_csp> vca_cli_csps
    = {{vca_colorSpace::YUV400, {1, {0, 0, 0}, {0, 0, 0}}},
       {vca_colorSpace::YUV420, {3, {0, 1, 1}, {0, 1, 1}}},
       {vca_colorSpace::YUV422, {3, {0, 1, 1}, {0, 0, 0}}},
       {vca_colorSpace::YUV444, {3, {0, 0, 0}, {0, 0, 0}}}};
