/*****************************************************************************
 * Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Christian Feldmann <christian.feldmann@bitmovin.com>
 *          Vignesh V Menon <vignesh.menon@aau.at>       
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

#include <common/getopt/getopt.h>

#include <stdio.h>

static const char short_options[]         = "f:h?";
static const struct option long_options[] = {{"help", no_argument, NULL, 'h'},
                                             {"iterations", required_argument, NULL, 'N'},
                                             {"input-res", required_argument, NULL, 0},
                                             {"input-depth", required_argument, NULL, 0},
                                             {"input-csp", required_argument, NULL, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0},
                                             {0, 0, 0, 0}};

static void showHelp()
{
    printf("\nSyntax: vcaPerformanceTest [options]\n");
    printf("    Run a test benchmark for the tool.\n");
    printf("\nExecutable Options:\n");
    printf("-h/--help                        Show this help text and exit\n");
    printf("\nOptions:\n");
    printf("-N/--iterations <integer>        How many frames should be pushed in each test. "
           "(Default 1000)\n");
    printf("   --input-res WxH               Test picture size [w x h] (Default 1920x1080)\n");
    printf("   --input-depth <integer>       Bit-depth of test input. Default 8\n");
    printf("   --input-csp <string>          Chroma subsampling for test\n");
    printf("                                 400 (4:0:0 monochrome)\n");
    printf("                                 420 (4:2:0 default)\n");
    printf("                                 422 (4:2:2)\n");
    printf("                                 444 (4:4:4)\n");
}
