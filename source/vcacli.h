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

#ifndef VCACLI_H
#define VCACLI_H 1

#include "common.h"
#include <getopt.h>

#ifdef __cplusplus
namespace VCA_NS {
#endif

static const char short_options[] = "o:D:P:p:f:F:r:I:i:b:s:t:q:m:hwV?";
static const struct option long_options[] =
{
    { "help",                 no_argument, NULL, 'h' },
    { "fullhelp",             no_argument, NULL, 0 },
    { "version",              no_argument, NULL, 'V' },
    { "asm",            required_argument, NULL, 0 },
    { "no-asm",               no_argument, NULL, 0 },
    { "pools",          required_argument, NULL, 0 },
    { "numa-pools",     required_argument, NULL, 0 },
    { "input",          required_argument, NULL, 0 },
    { "input-depth",    required_argument, NULL, 0 },
    { "input-res",      required_argument, NULL, 0 },
    { "input-csp",      required_argument, NULL, 0 },
    { "fps",            required_argument, NULL, 0 },
    { "seek",           required_argument, NULL, 0 },
    { "frame-skip",     required_argument, NULL, 0 },
    { "frames",         required_argument, NULL, 'f' },
    { "csv",            required_argument, NULL, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 }
};

static void showHelp(vca_param *param)
{
    int level = param->logLevel;

#define OPT(value) (value ? "enabled" : "disabled")
#define H0 printf
#define H1 if (level >= VCA_LOG_DEBUG) printf

    H0("\nSyntax: vca [options] infile [-o] outfile\n");
    H0("    infile can be YUV or Y4M\n");
    H0("\nExecutable Options:\n");
    H0("-h/--help                        Show this help text and exit\n");
    H0("   --fullhelp                    Show all options and exit\n");
    H0("-V/--version                     Show version info and exit\n");
    H0("\nInput Options:\n");
    H0("   --input <filename>            Raw YUV or Y4M input file name. `-` for stdin\n");
    H1("   --y4m                         Force parsing of input stream as YUV4MPEG2 regardless of file extension\n");
    H0("   --fps <float|rational>        Source frame rate (float or num/denom), auto-detected if Y4M\n");
    H0("   --input-res WxH               Source picture size [w x h], auto-detected if Y4M\n");
    H1("   --input-depth <integer>       Bit-depth of input file. Default 8\n");
    H1("   --input-csp <string>          Chroma subsampling, auto-detected if Y4M\n");
    H1("                                 0 - i400 (4:0:0 monochrome)\n");
    H1("                                 1 - i420 (4:2:0 default)\n");
    H1("                                 2 - i422 (4:2:2)\n");
    H1("                                 3 - i444 (4:4:4)\n");
    H0("-f/--frames <integer>            Maximum number of frames to encode. Default all\n");
    H0("   --seek <integer>              First frame to encode\n");
    H0("   --[no-]asm <bool|int|string>  Override CPU detection. Default: auto\n");
    H0("   --csv <filename>              Comma separated log file\n");
#undef OPT
#undef H0
#undef H1
    exit(1);
}

#ifdef __cplusplus
}
#endif

#endif
