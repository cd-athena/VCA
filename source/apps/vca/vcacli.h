/*****************************************************************************
 * Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
 *          Christian Feldmann <christian.feldmann@bitmovin.com>
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

#include <getopt.h>

#include <stdio.h>

static const char short_options[]         = "f:h?";
static const struct option long_options[] = {{"help", no_argument, NULL, 'h'},
                                             {"no-simd", no_argument, NULL, 0},
                                             {"no-dctenergy-chroma", no_argument, NULL, 0},
                                             {"no-entropy-chroma", no_argument, NULL, 0},
                                             {"no-lowpass", no_argument, NULL, 0},
                                             {"input", required_argument, NULL, 0},
                                             {"y4m", no_argument, NULL, 0},
                                             {"input-depth", required_argument, NULL, 0},
                                             {"input-res", required_argument, NULL, 0},
                                             {"input-csp", required_argument, NULL, 0},
                                             {"input-fps", required_argument, NULL, 0},
                                             {"skip", required_argument, NULL, 0},
                                             {"frames", required_argument, NULL, 'f'},
                                             {"complexity-csv", required_argument, NULL, 0},
                                             {"segment-size", required_argument, NULL, 0},
                                             {"segment-feature-csv", required_argument, NULL, 0},
                                             {"shot-csv", required_argument, NULL, 0},
                                             {"yuview-stats", required_argument, NULL, 0},
                                             {"max-epsthresh", required_argument, NULL, 0},
                                             {"min-epsthresh", required_argument, NULL, 0},
                                             {"max-sadthresh", required_argument, NULL, 0},
                                             {"block-size", required_argument, NULL, 0},
                                             {"threads", required_argument, NULL, 0},
                                             {"no-dctenergy", no_argument, 0},
                                             {"no-entropy", no_argument, 0},
                                             {"no-edgedensity", no_argument, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0}};

static void showHelp()
{
    printf("\nSyntax: vca [options] infile\n");
    printf("    infile can be YUV or Y4M\n");
    printf("\nExecutable Options:\n");
    printf("-h/--help                        Show this help text and exit\n");
    printf("\nInput Options:\n");
    printf("   --input <filename>            Raw YUV or Y4M input file name. `stdin` for stdin.");
    printf("   --y4m                         Force parsing of input stream as YUV4MPEG2 regardless "
           "of file extension\n");
    printf("   --input-res WxH               Source picture size [w x h], auto-detected if Y4M\n");
    printf("   --input-depth <integer>       Bit-depth of input file. Default 8\n");
    printf("   --input-csp <string>          Chroma subsampling, auto-detected if Y4M\n");
    printf("                                 400 (4:0:0 monochrome)\n");
    printf("                                 420 (4:2:0 default)\n");
    printf("                                 422 (4:2:2)\n");
    printf("                                 444 (4:4:4)\n");
    printf("   --input-fps <double>          Input fps, auto-detected if Y4M. Needed for shot "
           "detection.\n");
    printf("-f/--frames <integer>            Maximum number of frames to analyze. Default all\n");
    printf("   --skip <integer>              Skip N frames in the input before starting the "
           "analysis\n");
    printf("   --segment-size <integer>      Specifies the size of segment (Example: 1 = fps, 2 = "
            "2xfps)");
    printf("\nOutput Options:\n");
    printf("   --complexity-csv <filename>   Comma separated complexity log file\n");
    printf("   --segment-feature-csv <filename> Comma separated segment based complexity log "
            "file\n");
    printf("   --shot-csv <filename>         Comma separated shot detection log file.\n");
    printf("                                 Specify a filename to enable shot-detection.\n");
    printf("   --yuview-stats <filename>     Write the per block results (energy, sad) to a stats "
           "file\n");
    printf("                                 that can be visualized using YUView.\n");
    printf("\nOperation Options:\n");
    printf("   --no-simd                     Disable SIMD. Default: Enabled\n");
    printf("   --no-dctenergy-chroma         Disable chroma for DCT energy. Default: Enabled\n");
    printf("   --no-entropy-chroma           Disable chroma for entropy. Default: Enabled\n");
    printf("   --no-lowpass                  Disable lowpass DCT kernels. Default: Enabled\n");
    printf("   --max-epsthresh <float>       Maximum threshold of epsilon in shot detection\n");
    printf("   --min-epsthresh <float>       Minimum threshold of epsilon in shot detection\n");
    printf("   --min-sadthresh <float>       Minimum threshold of h in shot detection\n");
    printf("   --block-size <integer>        Block size for DCT transform. Must be 8, 16 or 32 "
           "(Default).\n");
    printf("   --threads <integer>           Nr of threads to use. (Default: 0 (autodetect))\n");
    printf("   --no-dctenergy                Disable DCT energy features. Default: Enabled\n");
    printf("   --no-entropy                  Disable entropy features. Default: Enabled\n");
    printf("   -no-edgedensity               Disable edge density calculation. Default: Enabled\n");
}
