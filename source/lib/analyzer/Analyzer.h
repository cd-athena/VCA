
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

#pragma once

#include <vcaLib.h>

namespace vca {

class Analyzer
{
public:
    Analyzer(vca_param param);
    ~Analyzer() = default;

    push_result pushFrame(const vca_frame *pic);

private:
    bool compute_weighted_DCT_energy(vca_frame *pic, vca_frame_texture_t *m_texture);
    void compute_dct_texture_SAD(double *normalizedTextureSad, vca_frame *pic);

    /* For DCT texture based complexity analysis */
    vca_frame_texture_t *cur_texture{};
    vca_frame_texture_t *prev_texture{};

    /* For DCT texture based scene-cut detection */
    double prev_norm_textureSad{};
    double *epsilons{};
    uint8_t *isNewShot{};
    int prevShotPos{};
    int *isNotSureShot{};
    int *prevShotDist{};

    vca_param param{};

    bool aborted{};
};
} // namespace vca
