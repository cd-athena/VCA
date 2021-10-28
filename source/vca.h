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

/* vca_encoder:
 *      opaque handler for encoder */
typedef struct vca_encoder vca_encoder;

/* vca_picyuv:
 *      opaque handler for PicYuv */
typedef struct vca_picyuv vca_picyuv;

typedef struct vca_frame_texture_t
{
    int32_t* m_ctuAbsoluteEnergy;
    double* m_ctuRelativeEnergy;
    int32_t m_variance;
    int32_t m_avgEnergy;
} vca_frame_texture_t;

/* Arbitrary User SEI
 * Payload size is in bytes and the payload pointer must be non-NULL. 
 * Payload types and syntax can be found in Annex D of the H.265 Specification.
 * SEI Payload Alignment bits as described in Annex D must be included at the 
 * end of the payload if needed. The payload should not be NAL-encapsulated.
 * Payloads are written in the order of input */

typedef enum
{
    BUFFERING_PERIOD                     = 0,
    PICTURE_TIMING                       = 1,
    PAN_SCAN_RECT                        = 2,
    FILLER_PAYLOAD                       = 3,
    USER_DATA_REGISTERED_ITU_T_T35       = 4,
    USER_DATA_UNREGISTERED               = 5,
    RECOVERY_POINT                       = 6,
    SCENE_INFO                           = 9,
    FULL_FRAME_SNAPSHOT                  = 15,
    PROGRESSIVE_REFINEMENT_SEGMENT_START = 16,
    PROGRESSIVE_REFINEMENT_SEGMENT_END   = 17,
    FILM_GRAIN_CHARACTERISTICS           = 19,
    POST_FILTER_HINT                     = 22,
    TONE_MAPPING_INFO                    = 23,
    FRAME_PACKING                        = 45,
    DISPLAY_ORIENTATION                  = 47,
    SOP_DESCRIPTION                      = 128,
    ACTIVE_PARAMETER_SETS                = 129,
    DECODING_UNIT_INFO                   = 130,
    TEMPORAL_LEVEL0_INDEX                = 131,
    DECODED_PICTURE_HASH                 = 132,
    SCALABLE_NESTING                     = 133,
    REGION_REFRESH_INFO                  = 134,
    MASTERING_DISPLAY_INFO               = 137,
    CONTENT_LIGHT_LEVEL_INFO             = 144,
    ALTERNATIVE_TRANSFER_CHARACTERISTICS = 147,
} SEIPayloadType;

typedef struct vca_sei_payload
{
    int payloadSize;
    SEIPayloadType payloadType;
    uint8_t* payload;
} vca_sei_payload;

typedef struct vca_sei
{
    int numPayloads;
    vca_sei_payload *payloads;
} vca_sei;

typedef struct vca_dolby_vision_rpu
{
    int payloadSize;
    uint8_t* payload;
}vca_dolby_vision_rpu;

/* Frame level statistics */
typedef struct vca_frame_stats
{
    int              poc;
    int32_t          e_value;
    double           h_value;
    double           epsilon;
} vca_frame_stats;

/* Used to pass pictures into the encoder, and to get picture data back out of
 * the encoder.  The input and output semantics are different */
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
     * the encoder's internal bit depth, but this field must describe the depth
     * of the input pictures. Must be between 8 and 16. Values larger than 8
     * imply 16bits per input sample. If input bit depth is larger than the
     * internal bit depth, the encoder will down-shift pixels. Input samples
     * larger than 8bits will be masked to internal bit depth. On output the
     * bitDepth will be the internal encoder bit depth */
    int     bitDepth;

    /* Ignored on input, set to picture count, returned on output */
    int     poc;

    /* Must be specified on input pictures: VCA_CSP_I420 or other. It must
     * match the internal color space of the encoder. vca_picture_init() will
     * initialize this value to the internal color space */
    int     colorSpace;

    /* Frame level statistics */
    vca_frame_stats frameData;

    uint64_t framesize;

    int    height;
} vca_picture;

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
    int              logLevel;               /* The level of logging detail emitted by the encoder */

    /*== Internal Picture Specification ==*/
    int              internalBitDepth;       /* Internal encoder bit depth */
    int              internalCsp;            /* Color space of internal pictures */
    uint32_t         fpsNum;                 /* Numerator of frame rate */
    uint32_t         fpsDenom;               /* Denominator of frame rate */
    int              sourceWidth;            /* Width (in pixels) of the source pictures */
    int              sourceHeight;           /* Height (in pixels) of the source pictures */
    int              totalFrames;            /* Total Number of frames to be encoded */
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

#define VCA_MAJOR_VERSION 1

vca_param*       param_alloc();
void             param_free(vca_param*);
void             param_default(vca_param*);
int              param_parse(vca_param*, const char*, const char*);
vca_picture*     picture_alloc(void);
void             picture_free(vca_picture*);
void             picture_init(vca_param*, vca_picture*);
vca_encoder*     encoder_open(vca_param*);
void             encoder_parameters(vca_encoder*, vca_param*);
int              encoder_encode(vca_encoder*, vca_picture*);
void             encoder_close(vca_encoder*);
void             dither_image(vca_picture*, int, int, int16_t*, int);
FILE*            csv_E_h_log_open(const vca_param* param);
FILE*            csv_shot_log_open(const vca_param* param);
void             csv_E_h_log_frame(const vca_param* param, const vca_picture* pic);
void             encoder_shot_detect(vca_encoder*);
void             encoder_shot_print(vca_encoder*);

int              check_params(vca_param *param);
void             print_params(vca_param *param);
int              vca_atoi(const char *str, bool& bError);
double           vca_atof(const char *str, bool& bError);
void             setParamAspectRatio(vca_param *p, int width, int height);
void             getParamAspectRatio(vca_param *p, int& width, int& height);

#ifdef __cplusplus
}
#endif

#endif // VCA_H
