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

#ifndef VCA_ENCODER_H
#define VCA_ENCODER_H

#include "common.h"
#include "threading.h"

struct vca_analyzer {};
namespace VCA_NS {
// private namespace
extern const char g_sliceTypeToChar[3];

class Analyzer : public vca_analyzer
{
public:
    int64_t                m_analyzeStartTime;
    vca_param*             m_param;
    bool                   m_aborted;          // fatal error detected

    /* For DCT texture based complexity analysis */
    vca_frame_texture_t    *cur_texture;
    vca_frame_texture_t    *prev_texture;

    /* For DCT texture based scene-cut detection */
    double                 prev_norm_textureSad;
    double                 *epsilons;
    uint8_t                *isNewShot;
    int                    prevShotPos;
    int                    *isNotSureShot;
    int                    *prevShotDist;
    Analyzer();
    ~Analyzer()
    {
    };

    void create();
    void destroy();

    int analyze(const vca_picture* pic);
    bool compute_weighted_DCT_energy(vca_picture *pic, vca_frame_texture_t *m_texture);
    void compute_dct_texture_SAD(double *normalizedTextureSad, vca_picture *pic);
    void configure(vca_param *param);
};
}

#endif // ifndef VCA_ENCODER_H
