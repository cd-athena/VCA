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
#include "vca.h"

struct vca_encoder {};
namespace VCA_NS {
// private namespace
extern const char g_sliceTypeToChar[3];

class Encoder : public vca_encoder
{
public:
    int64_t            m_firstPts;
    int64_t            m_encodeStartTime;
    int                m_pocLast;         // time index (POC)
    int                m_encodedFrameNum;
    int                m_outputCount;
    int                m_curEncoder;
    vca_param*        m_param;
    bool               m_aborted;          // fatal error detected
    bool               m_reconfigure;      // Encoder reconfigure in progress

    /* For DCT texture based scene-cut detection */
    vca_frame_texture_t *cur_texture;
    vca_frame_texture_t *prev_texture;
    double             prev_norm_textureSad;

    Encoder();
    ~Encoder()
    {
    };

    void create();
    void destroy();

    int encode(const vca_picture* pic);
    bool compute_weighted_DCT_energy(vca_picture *pic, vca_frame_texture_t *m_texture);
    void compute_dct_texture_SAD(double *normalizedTextureSad, vca_picture *pic);
    void configure(vca_param *param);
};
}

#endif // ifndef VCA_ENCODER_H
