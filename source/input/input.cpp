/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
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

#include "input.h"
#include "yuv.h"
#include "y4m.h"

using namespace VCA_NS;

InputFile* InputFile::open(InputFileInfo& info, bool bForceY4m)
{
    const char * s = strrchr(info.filename, '.');

    if (bForceY4m || (s && !strcmp(s, ".y4m")))
        return new Y4MInput(info);
    else
        return new YUVInput(info);
}
