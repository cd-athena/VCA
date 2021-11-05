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

#ifndef VCA_H
#define VCA_H
#include <stdint.h>
#include <stdio.h>
#include "vca_config.h"
#ifdef __cplusplus
extern "C" {
#endif

#if _MSC_VER
#pragma warning(disable: 4201) // non-standard extension used (nameless struct/union)
#endif

/* vca_analyzer:
 *      opaque handler for analyzer */
typedef struct vca_analyzer vca_analyzer;

/* vca_picyuv:
 *      opaque handler for PicYuv */
typedef struct vca_picyuv vca_picyuv;

/* Application developers planning to link against a shared library version of
 * libvca from a Microsoft Visual Studio or similar development environment
 * will need to define VCA_API_IMPORTS before including this header.
 * This clause does not apply to MinGW, similar development environments, or non
 * Windows platforms. */
#ifdef VCA_API_IMPORTS
#define VCA_API __declspec(dllimport)
#else
#define VCA_API
#endif

typedef struct vca_frame_texture_t
{
    int32_t* m_ctuAbsoluteEnergy;
    double* m_ctuRelativeEnergy;
    int32_t m_variance;
    int32_t m_avgEnergy;
} vca_frame_texture_t;

/* Frame level statistics */
typedef struct vca_frame_stats
{
    int              poc;
    int32_t          e_value;
    double           h_value;
    double           epsilon;
} vca_frame_stats;

/* Used to pass pictures into the analyzer, and to get picture data back out of
 * the analyzer.  The input and output semantics are different */
typedef struct vca_picture
{
    /* presentation time stamp: user-specified, returned on output */
    int64_t pts;

    /* display time stamp: ignored on input, copied from reordered pts. Returned
     * on output */
    int64_t dts;

    /* Must be specified on input pictures, the number of planes is determined
     * by the colorSpace value */
    void*   planes[3];

    /* Stride is the number of bytes between row starts */
    int     stride[3];

    /* Must be specified on input pictures. vca_picture_init() will set it to
     * the analyzer's internal bit depth, but this field must describe the depth
     * of the input pictures. Must be between 8 and 16. Values larger than 8
     * imply 16bits per input sample. If input bit depth is larger than the
     * internal bit depth, the analyzer will down-shift pixels. Input samples
     * larger than 8bits will be masked to internal bit depth. On output the
     * bitDepth will be the internal analyzer bit depth */
    int     bitDepth;

    /* Ignored on input, set to picture count, returned on output */
    int     poc;

    /* Must be specified on input pictures: VCA_CSP_I420 or other. It must
     * match the internal color space of the analyzer. vca_picture_init() will
     * initialize this value to the internal color space */
    int     colorSpace;

    /* Frame level statistics */
    vca_frame_stats frameData;

    uint64_t framesize;

    int    height;
} vca_picture;

/* x86 CPU flags */
#define VCA_CPU_MMX             (1 << 0)
#define VCA_CPU_MMX2            (1 << 1)  /* MMX2 aka MMXEXT aka ISSE */
#define VCA_CPU_MMXEXT          VCA_CPU_MMX2
#define VCA_CPU_SSE             (1 << 2)
#define VCA_CPU_SSE2            (1 << 3)
#define VCA_CPU_LZCNT           (1 << 4)
#define VCA_CPU_SSE3            (1 << 5)
#define VCA_CPU_SSSE3           (1 << 6)
#define VCA_CPU_SSE4            (1 << 7)  /* SSE4.1 */
#define VCA_CPU_SSE42           (1 << 8)  /* SSE4.2 */
#define VCA_CPU_AVX             (1 << 9)  /* Requires OS support even if YMM registers aren't used. */
#define VCA_CPU_XOP             (1 << 10)  /* AMD XOP */
#define VCA_CPU_FMA4            (1 << 11)  /* AMD FMA4 */
#define VCA_CPU_FMA3            (1 << 12)  /* Intel FMA3 */
#define VCA_CPU_BMI1            (1 << 13)  /* BMI1 */
#define VCA_CPU_BMI2            (1 << 14)  /* BMI2 */
#define VCA_CPU_AVX2            (1 << 15)  /* AVX2 */
#define VCA_CPU_AVX512          (1 << 16)  /* AVX-512 {F, CD, BW, DQ, VL}, requires OS support */
/* x86 modifiers */
#define VCA_CPU_CACHELINE_32    (1 << 17)  /* avoid memory loads that span the border between two cachelines */
#define VCA_CPU_CACHELINE_64    (1 << 18)  /* 32/64 is the size of a cacheline in bytes */
#define VCA_CPU_SSE2_IS_SLOW    (1 << 19)  /* avoid most SSE2 functions on Athlon64 */
#define VCA_CPU_SSE2_IS_FAST    (1 << 20)  /* a few functions are only faster on Core2 and Phenom */
#define VCA_CPU_SLOW_SHUFFLE    (1 << 21)  /* The Conroe has a slow shuffle unit (relative to overall SSE performance) */
#define VCA_CPU_STACK_MOD4      (1 << 22)  /* if stack is only mod4 and not mod16 */
#define VCA_CPU_SLOW_ATOM       (1 << 23)  /* The Atom is terrible: slow SSE unaligned loads, slow
                                             * SIMD multiplies, slow SIMD variable shifts, slow pshufb,
                                             * cacheline split penalties -- gather everything here that
                                             * isn't shared by other CPUs to avoid making half a dozen
                                             * new SLOW flags. */
#define VCA_CPU_SLOW_PSHUFB     (1 << 24)  /* such as on the Intel Atom */
#define VCA_CPU_SLOW_PALIGNR    (1 << 25)  /* such as on the AMD Bobcat */

/* Log level */
#define VCA_LOG_NONE          (-1)
#define VCA_LOG_ERROR          0
#define VCA_LOG_WARNING        1
#define VCA_LOG_INFO           2
#define VCA_LOG_DEBUG          3
#define VCA_LOG_FULL           4

#define VCA_MAX_FRAME_THREADS  16

/* Supported internal color space types (according to semantics of chroma_format_idc) */
#define VCA_CSP_I400           0  /* yuv 4:0:0 planar */
#define VCA_CSP_I420           1  /* yuv 4:2:0 planar */
#define VCA_CSP_I422           2  /* yuv 4:2:2 planar */
#define VCA_CSP_I444           3  /* yuv 4:4:4 planar */
#define VCA_CSP_COUNT          4  /* Number of supported internal color spaces */

/* Interleaved color-spaces may eventually be supported as input pictures */
#define VCA_CSP_BGR            6  /* packed bgr 24bits   */
#define VCA_CSP_BGRA           7  /* packed bgr 32bits   */
#define VCA_CSP_RGB            8  /* packed rgb 24bits   */
#define VCA_CSP_MAX            9  /* end of list */
#define VCA_EXTENDED_SAR       255 /* aspect ratio explicitly specified as width:height */

typedef struct vca_cli_csp
{
    int planes;
    int width[3];
    int height[3];
} vca_cli_csp;

static const vca_cli_csp vca_cli_csps[] =
{
    { 1, { 0, 0, 0 }, { 0, 0, 0 } }, /* i400 */
    { 3, { 0, 1, 1 }, { 0, 1, 1 } }, /* i420 */
    { 3, { 0, 1, 1 }, { 0, 0, 0 } }, /* i422 */
    { 3, { 0, 0, 0 }, { 0, 0, 0 } }, /* i444 */
    { 2, { 0, 0 },    { 0, 1 } },    /* nv12 */
    { 2, { 0, 0 },    { 0, 0 } },    /* nv16 */
};

/* String values accepted by vca_param_parse() (and CLI) for various parameters */
static const char * const vca_source_csp_names[] = { "i400", "i420", "i422", "i444", "nv12", "nv16", 0 };

struct vca_param;

/* vca input parameters
 *
 * For version safety you may use vca_param_alloc/free() to manage the
 * allocation of vca_param instances, and vca_param_parse() to assign values
 * by name.  By never dereferencing param fields in your own code you can treat
 * vca_param as an opaque data structure */
typedef struct vca_param
{
    int              cpuid;

    /*== Parallelism Features ==*/
    int              frameNumThreads;
    const char*      numaPools;
    int              bEnableWavefront;       /* Enable wavefront parallel processing of DCT energies */
    int              bEnableShotdetect;      /* Enable shot detection algorithm using epsilon feature */
    /*== Logging Feature ==*/
    int              logLevel;               /* The level of logging detail emitted by the analyzer */

    /*== Internal Picture Specification ==*/
    int              internalBitDepth;       /* Internal analyzer bit depth */
    int              internalCsp;            /* Color space of internal pictures */
    uint32_t         fpsNum;                 /* Numerator of frame rate */
    uint32_t         fpsDenom;               /* Denominator of frame rate */
    int              sourceWidth;            /* Width (in pixels) of the source pictures */
    int              sourceHeight;           /* Height (in pixels) of the source pictures */
    int              totalFrames;            /* Total Number of frames to be analyzed */
    uint32_t         maxCUSize;              /* Maximum CU width and height in pixels */

    /*== Video Usability Information ==*/
    struct
    {
        int          aspectRatioIdc;         /* Aspect ratio idc to be added to the VUI */
        int          sarWidth;               /* Sample Aspect Ratio width in arbitrary units to be added to the VUI */
        int          sarHeight;              /* Sample Aspect Ratio height in arbitrary units to be added to the VUI */
    } vui;

    uint16_t         maxCLL;                 /* Maximum Content light level(MaxCLL) */
    uint16_t         maxFALL;                /* Maximum Frame Average Light Level(MaxFALL) */
    uint16_t         minLuma;                /* Minimum luma level of input source picture */
    uint16_t         maxLuma;                /* Maximum luma level of input source picture */
    uint32_t         maxLog2CUSize;          /* Log of maximum CTU size */
    int              bLowPassDct;            /* Use low-pass subband dct approximation */
    const char*      csv_E_h_fn;             /* filename of E-h metrics CSV log */
    FILE*            csv_E_h_fpt;            /* File pointer for E-h metrics csv log */
    const char*      csv_shot_fn;             /* filename of E-h metrics CSV log */
    FILE*            csv_shot_fpt;            /* File pointer for E-h metrics csv log */

    double           minThresh;              /* Minimum threshold for epsilon in shot detection */
    double           maxThresh;              /* Maximum threshold for epsilon in shot detection */
} vca_param;

#define VCA_PARAM_BAD_NAME  (-1)
#define VCA_PARAM_BAD_VALUE (-2)

VCA_API extern const int vca_max_bit_depth;
VCA_API extern const char *vca_version_str;
VCA_API extern const char *vca_build_info_str;

/* Force a link error in the case of linking against an incompatible API version.
 * Glue #defines exist to force correct macro expansion; the final output of the macro
 * is x265_analyzer_open_##X265_BUILD (for purposes of dlopen). */
#define vca_analyzer_glue1(x, y) x ## y
#define vca_analyzer_glue2(x, y) vca_analyzer_glue1(x, y)
#define vca_analyzer_open vca_analyzer_glue2(vca_analyzer_open_, VCA_BUILD)

vca_analyzer*     vca_analyzer_open(vca_param *);
void             vca_analyzer_parameters(vca_analyzer *enc, vca_param *out);
int              vca_analyzer_analyze(vca_analyzer *enc, vca_picture *pic_in);
void             vca_analyzer_close(vca_analyzer *enc);
void             vca_analyzer_shot_detect(vca_analyzer *enc);
void             vca_analyzer_shot_print(vca_analyzer *enc);
vca_picture*     vca_picture_alloc();
void             vca_picture_init(vca_param *param, vca_picture *pic);
void             vca_picture_free(vca_picture *p);
FILE*            vca_csv_E_h_log_open(const vca_param* param);
FILE*            vca_csv_shot_log_open(const vca_param* param);
void             vca_csv_E_h_log_frame(const vca_param* param, const vca_picture* pic);

#define VCA_MAJOR_VERSION 1

vca_param*       param_alloc();
void             param_free(vca_param*);
void             param_default(vca_param*);
int              param_parse(vca_param*, const char*, const char*);
void             dither_image(vca_picture*, int, int, int16_t*, int);

int              check_params(vca_param *param);
void             print_params(vca_param *param);
int              vca_atoi(const char *str, bool& bError);
double           vca_atof(const char *str, bool& bError);
void             setParamAspectRatio(vca_param *p, int width, int height);
void             getParamAspectRatio(vca_param *p, int& width, int& height);

/* === Multi-lib API === */

typedef struct vca_api
{
    int           api_major_version;    /* VCA_MAJOR_VERSION */
    int           api_build_number;     /* VCA_BUILD (soname) */
    int           sizeof_param;         /* sizeof(vca_param) */
    int           sizeof_picture;       /* sizeof(vca_picture) */

    int           bit_depth;
    const char*   version_str;
    const char*   build_info_str;

    /* libvca public API functions, documented above with vca_ prefixes */
    vca_analyzer*  (*analyzer_open)(vca_param*);
    void          (*analyzer_parameters)(vca_analyzer *enc, vca_param *out);
    int           (*analyzer_analyze)(vca_analyzer *enc, vca_picture *pic_in);
    void          (*analyzer_close)(vca_analyzer *enc);
    void          (*analyzer_shot_detect)(vca_analyzer *enc);
    void          (*analyzer_shot_print)(vca_analyzer *enc);
    vca_picture*  (*picture_alloc)();
    void          (*picture_init)(vca_param *param, vca_picture *pic);
    void          (*picture_free)(vca_picture *p);
    FILE*         (*csv_E_h_log_open)(const vca_param* param);
    FILE*         (*csv_shot_log_open)(const vca_param* param);
    void          (*csv_E_h_log_frame)(const vca_param* param, const vca_picture* pic);
    /* add new pointers to the end, or increment VCA_MAJOR_VERSION */
} vca_api;

#define vca_api_glue1(x, y) x ## y
#define vca_api_glue2(x, y) vca_api_glue1(x, y)
#define vca_api_get vca_api_glue2(vca_api_get_, VCA_BUILD)

const vca_api* vca_api_get(int bitDepth);

const vca_api* vca_api_query(int bitDepth, int apiVersion, int* err);

#define VCA_API_QUERY_ERR_NONE           0 /* returned API pointer is non-NULL */
#define VCA_API_QUERY_ERR_VER_REFUSED    1 /* incompatible version skew        */
#define VCA_API_QUERY_ERR_LIB_NOT_FOUND  2 /* libvca_main10 not found, for ex */
#define VCA_API_QUERY_ERR_FUNC_NOT_FOUND 3 /* unable to bind vca_api_query    */
#define VCA_API_QUERY_ERR_WRONG_BITDEPTH 4 /* libvca_main10 not 10bit, for ex */

static const char * const vca_api_query_errnames[] = {
    "api queried from libvca",
    "libvca version is not compatible with this application",
    "unable to bind a libvca with requested bit depth",
    "unable to bind vca_api_query from libvca",
    "libvca has an invalid bitdepth"
};

#ifdef __cplusplus
}
#endif

#endif // VCA_H
