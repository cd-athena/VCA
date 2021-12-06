
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

static const std::map<vca_colorSpace, vca_cli_csp> vca_cli_csps =
{
    {vca_colorSpace::YUV400, { 1, { 0, 0, 0 }, { 0, 0, 0 } } },
    {vca_colorSpace::YUV420, { 3, { 0, 1, 1 }, { 0, 1, 1 } } },
    {vca_colorSpace::YUV422, { 3, { 0, 1, 1 }, { 0, 0, 0 } } },
    {vca_colorSpace::YUV444, { 3, { 0, 0, 0 }, { 0, 0, 0 } } }
};

