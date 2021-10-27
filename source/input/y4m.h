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

#ifndef VCA_Y4M_H
#define VCA_Y4M_H

#include "input.h"
#include "threading.h"
#include <fstream>

#define QUEUE_SIZE 5

namespace VCA_NS {
// vca private namespace

class Y4MInput : public InputFile, public Thread
{
protected:

    uint32_t rateNum;

    uint32_t rateDenom;

    uint32_t sarWidth;

    uint32_t sarHeight;

    size_t framesize;

    int depth;

    int width;

    int height;

    int colorSpace;

    bool threadActive;

    ThreadSafeInteger readCount;

    ThreadSafeInteger writeCount;
    char* buf[QUEUE_SIZE];
    FILE *ifs;
    bool parseHeader();
    void threadMain();

    bool populateFrameQueue();

public:

    Y4MInput(InputFileInfo& info);

    virtual ~Y4MInput();
    void release();
    bool isEof() const            { return ifs && feof(ifs); }
    bool isFail()                 { return !(ifs && !ferror(ifs) && threadActive); }
    void startReader();
    bool readPicture(vca_picture&);

    const char *getName() const   { return "y4m"; }

    int getWidth() const                          { return width; }

    int getHeight() const                         { return height; }
};
}

#endif // ifndef VCA_Y4M_H
