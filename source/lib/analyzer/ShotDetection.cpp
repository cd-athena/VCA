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

#include "ShotDetection.h"

#include <string>
#include <vector>

namespace {

inline void log(const vca_shot_detection_param &cfg, LogLevel level, const std::string &message)
{
    if (cfg.logFunction)
        cfg.logFunction(cfg.logFunctionPrivateData, level, message.c_str());
}

void detect(const vca_shot_detection_param &param, vca_frame_results *frames, size_t num_frames)
{
    struct UnsureFrame
    {
        size_t index{};
        size_t previousShotDistance{};
    };
    std::vector<UnsureFrame> unsureFrames;

    unsigned numDetectedShots = 0;

    /* First pass */
    size_t prevShotPos    = 0;
    size_t not_sure_count = 0;
    for (size_t i = 2; i < num_frames; i++)
    {
        if (frames[i].epsilon > param.maxEpsilonThresh)
        {
            frames[i].isNewShot = true;
            prevShotPos         = i;
            numDetectedShots++;
        }
        else
        {
            frames[i].isNewShot = false;
            if (frames[i].epsilon >= param.minEpsilonThresh && frames[i].energyDiff >=param.maxSadThresh)
            {
                auto previousShotDistance = i - prevShotPos;
                unsureFrames.push_back({i, previousShotDistance});
            }
        }
    }

    log(param,
        LogLevel::Debug,
        "First pass complete. " + std::to_string(unsureFrames.size()) + " frames not decided yet.");

    for (auto it = unsureFrames.begin(); it != unsureFrames.end(); it++)
    {
        auto itNext = it + 1;
        if (itNext != unsureFrames.end() && it->previousShotDistance >= param.fps
            && (itNext->index - it->index) >= param.fps)
        {
            frames[it->index].isNewShot = true;
            numDetectedShots++;
        }

        if (it->index == unsureFrames.back().index && it->previousShotDistance >= param.fps
            && (it->index + param.fps) <= num_frames)
        {
            frames[it->index].isNewShot = true;
            numDetectedShots++;
        }
    }

    log(param, LogLevel::Debug, "Detected " + std::to_string(numDetectedShots) + " shots.");
}

} // namespace

namespace vca {

vca_result shot_detection(const vca_shot_detection_param &param,
                          vca_frame_results *frames,
                          size_t num_frames)
{
    log(param,
        LogLevel::Info,
        "Starting shot detection for " + std::to_string(num_frames) + " frames");

    frames[0].isNewShot = true;

    if (num_frames < 3)
        return vca_result::VCA_OK;

    try
    {
        detect(param, frames, num_frames);
    }
    catch (const std::exception &e)
    {
        std::string exception_str = e.what();
        log(param, LogLevel::Error, "Exception " + exception_str);
        return vca_result::VCA_ERROR;
    }

    return vca_result::VCA_OK;
}

} // namespace vca
