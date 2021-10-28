/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Loren Merritt <lorenm@u.washington.edu>
 *          Steve Borho <steve@borho.org>
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

#ifndef VCA_CPU_H
#define VCA_CPU_H

#include "common.h"

// from primitives.cpp
extern "C" void vca_cpu_emms(void);

#if _MSC_VER
#include <mmintrin.h>
#define vca_emms() _mm_empty()
#elif __GNUC__
// Cannot use _mm_empty() directly without compiling all the source with
// a fixed CPU arch, which we would like to avoid at the moment
#define vca_emms() vca_cpu_emms()
#else
#define vca_emms() vca_cpu_emms()
#endif

namespace VCA_NS {

uint32_t cpu_detect();

struct cpu_name_t
{
    char name[16];
    uint32_t flags;
};

extern const cpu_name_t cpu_names[];
}

#endif // ifndef VCA_CPU_H
