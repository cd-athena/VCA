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

#include "encoder.h"

#if EXPORT_C_API
/* these functions are exported as C functions (default) */
using namespace VCA_NS;
extern "C" {
#else
/* these functions exist within private namespace (multilib) */
namespace X265_NS {
#endif

    vca_encoder* vca_encoder_open(vca_param *p)
    {
        if (!p)
            return NULL;

#if _MSC_VER
#pragma warning(disable: 4127) // conditional expression is constant, yes I know
#endif

#if HIGH_BIT_DEPTH
        if (VCA_DEPTH != 10 && VCA_DEPTH != 12)
#else
        if (VCA_DEPTH != 8)
#endif
        {
            vca_log(p, VCA_LOG_ERROR, "Build error, internal bit depth mismatch\n");
            return NULL;
        }

        Encoder* encoder = NULL;
        vca_param* param = param_alloc();
        if (!param)
            goto fail;

        memcpy(param, p, sizeof(vca_param));

        vca_setup_primitives(param);

        if (check_params(param))
            goto fail;

        encoder = new Encoder;

        // may change params for auto-detect, etc
        encoder->configure(param);

        encoder->create();

        /* Try to open complexity CSV file handle */
        if (encoder->m_param->csv_E_h_fn)
        {
            encoder->m_param->csv_E_h_fpt = vca_csv_E_h_log_open(encoder->m_param);
            if (!encoder->m_param->csv_E_h_fpt)
            {
                vca_log(encoder->m_param, VCA_LOG_ERROR, "Unable to open complexity CSV log file <%s>, aborting\n", encoder->m_param->csv_E_h_fn);
                encoder->m_aborted = true;
            }
        }
        /* Try to open shot detection CSV file handle */
        if (encoder->m_param->csv_shot_fn)
        {
            encoder->m_param->csv_shot_fpt = vca_csv_shot_log_open(encoder->m_param);
            if (!encoder->m_param->csv_shot_fpt)
            {
                vca_log(encoder->m_param, VCA_LOG_ERROR, "Unable to open shot detection CSV log file <%s>, aborting\n", encoder->m_param->csv_shot_fn);
                encoder->m_aborted = true;
            }
        }
        encoder->m_param = param;
        memcpy(encoder->m_param, param, sizeof(vca_param));
        if (encoder->m_aborted)
            goto fail;

        print_params(param);
        return encoder;

    fail:
        delete encoder;
        param_free(param);
        return NULL;
    }

    void vca_encoder_parameters(vca_encoder *enc, vca_param *out)
    {
        if (enc && out)
        {
            Encoder *encoder = static_cast<Encoder*>(enc);
            memcpy(out, encoder->m_param, sizeof(vca_param));
        }
    }

    int vca_encoder_encode(vca_encoder *enc, vca_picture *pic_in)
    {
        if (!enc)
            return -1;

        if (!pic_in)
            return 0;

        Encoder *encoder = static_cast<Encoder*>(enc);
        int numEncoded;

        do
        {
            numEncoded = encoder->encode(pic_in);
            if (numEncoded)
                vca_csv_E_h_log_frame(encoder->m_param, pic_in);
        } while (numEncoded == 0 && !pic_in);

        if (numEncoded < 0)
            encoder->m_aborted = true;

        return numEncoded;
    }

    void vca_encoder_close(vca_encoder *enc)
    {
        if (enc)
        {
            Encoder *encoder = static_cast<Encoder*>(enc);
            encoder->destroy();
            delete encoder;
        }
    }

    void vca_encoder_shot_detect(vca_encoder *enc)
    {
        Encoder *encoder = static_cast<Encoder*>(enc);
        int numFrames = encoder->m_param->totalFrames;

        /* First pass */
        encoder->isNewShot[0] = VCA_NEW_SHOT;
        encoder->isNewShot[1] = VCA_NOT_NEW_SHOT;
        encoder->prevShotPos = 0;
        int not_sure_count = 0;
        for (int i = 2, j = 0; i < numFrames; i++)
        {
            if (encoder->epsilons[i] > encoder->m_param->maxThresh)
            {
                encoder->isNewShot[i] = VCA_NEW_SHOT;
                encoder->prevShotPos = i;
            }
            else if (encoder->epsilons[i] < encoder->m_param->minThresh)
            {
                encoder->isNewShot[i] = VCA_NOT_NEW_SHOT;
            }
            else
            {
                encoder->isNewShot[i] = VCA_NOTSURE_NEW_SHOT;
                encoder->isNotSureShot[j] = i;
                encoder->prevShotDist[j] = i - encoder->prevShotPos;
                not_sure_count++;
                j++;
            }
        }
        /* If there are no VCA_NOTSURE_NEW_SHOT entries, shot detection has been completed */
        vca_log(encoder->m_param, VCA_LOG_DEBUG, "First pass of shot detection complete!\n");
        if (!not_sure_count)
            return;


        /* Second pass */
        for (int j = 0; j < not_sure_count; j++)
        {
            int fps_value = encoder->m_param->fpsNum / encoder->m_param->fpsDenom;
            if (encoder->prevShotDist[j] > fps_value && encoder->prevShotDist[j + 1] > fps_value)
            {
                encoder->isNewShot[encoder->isNotSureShot[j]] = VCA_NEW_SHOT;
            }
            else
            {
                encoder->isNewShot[encoder->isNotSureShot[j]] = VCA_NOT_NEW_SHOT;
            }
        }
        vca_log(encoder->m_param, VCA_LOG_DEBUG, "Second pass of shot detection complete!\n");
    }

    void vca_encoder_shot_print(vca_encoder *enc)
    {
        Encoder *encoder = static_cast<Encoder*>(enc);
        int numFrames = encoder->m_param->totalFrames;
        int shot_count = 1;
        for (int i = 0; i < numFrames; i++)
        {
            if (encoder->isNewShot[i] == VCA_NEW_SHOT)
            {
                if (encoder->m_param->csv_shot_fpt)
                {
                    if (i)
                        fprintf(encoder->m_param->csv_shot_fpt, "%d,\n", i - 1);
                    fprintf(encoder->m_param->csv_shot_fpt, "%d, %d,", shot_count, i);
                    shot_count++;
                    fflush(stderr);
                }
                vca_log(encoder->m_param, VCA_LOG_INFO, "IDR at POC %d\n", i);
            }
            if (encoder->m_param->csv_shot_fpt)
            {
                if (i == (numFrames - 1))
                    fprintf(encoder->m_param->csv_shot_fpt, "%d,\n", numFrames - 1);
            }
        }
    }

    vca_picture *vca_picture_alloc()
    {
        return (vca_picture*)vca_malloc(sizeof(vca_picture));
    }

    void vca_picture_init(vca_param *param, vca_picture *pic)
    {
        memset(pic, 0, sizeof(vca_picture));
        pic->bitDepth = param->internalBitDepth;
        pic->colorSpace = param->internalCsp;
    }

    void vca_picture_free(vca_picture *p)
    {
        return vca_free(p);
    }

    FILE* vca_csv_E_h_log_open(const vca_param* param)
    {
        FILE *csvfp = vca_fopen(param->csv_E_h_fn, "r");
        if (csvfp)
        {
            /* file already exists, re-open for append */
            fclose(csvfp);
            return vca_fopen(param->csv_E_h_fn, "ab");
        }
        else
        {
            /* new CSV file, write header */
            csvfp = vca_fopen(param->csv_E_h_fn, "wb");
            if (csvfp)
            {
                fprintf(csvfp, "POC, E, h,");
                if (param->bEnableShotdetect)
                    fprintf(csvfp, "epsilon,");
                fprintf(csvfp, "\n");
            }
            return csvfp;
        }
    }

    FILE* vca_csv_shot_log_open(const vca_param* param)
    {
        FILE *csvfp = vca_fopen(param->csv_shot_fn, "r");
        if (csvfp)
        {
            /* file already exists, re-open for append */
            fclose(csvfp);
            return vca_fopen(param->csv_shot_fn, "ab");
        }
        else
        {
            /* new CSV file, write header */
            csvfp = vca_fopen(param->csv_shot_fn, "wb");
            if (csvfp)
            {
                fprintf(csvfp, "Shot ID, Start POC, End POC, ");
                fprintf(csvfp, "\n");
            }
            return csvfp;
        }
    }

    void vca_csv_E_h_log_frame(const vca_param* param, const vca_picture* pic)
    {
        if (!param->csv_E_h_fpt)
            return;

        const vca_frame_stats* frameStats = &pic->frameData;
        fprintf(param->csv_E_h_fpt, "%d, %d, %f,", frameStats->poc, frameStats->e_value, frameStats->h_value);
        if (param->bEnableShotdetect)
            fprintf(param->csv_E_h_fpt, "%f,", frameStats->epsilon);
        fprintf(param->csv_E_h_fpt, "\n");
        fflush(stderr);
    }

    static const vca_api libapi =
    {
        VCA_MAJOR_VERSION,
        VCA_BUILD,
        sizeof(vca_param),
        sizeof(vca_picture),

        vca_max_bit_depth,
        vca_version_str,
        vca_build_info_str,

        &vca_encoder_open,
        &vca_encoder_parameters,
        &vca_encoder_encode,
        &vca_encoder_close,
        &vca_encoder_shot_detect,
        &vca_encoder_shot_print,
        &vca_picture_alloc,
        &vca_picture_init,
        &vca_picture_free,
        &vca_csv_E_h_log_open,
        &vca_csv_shot_log_open,
        &vca_csv_E_h_log_frame,
    };

    typedef const vca_api* (*api_get_func)(int bitDepth);
    typedef const vca_api* (*api_query_func)(int bitDepth, int apiVersion, int* err);

#define xstr(s) str(s)
#define str(s) #s

#if _WIN32
#define ext ".dll"
#elif MACOS
#include <dlfcn.h>
#define ext ".dylib"
#else
#include <dlfcn.h>
#define ext ".so"
#endif
#if defined(__GNUC__) && __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif

    static int g_recursion /* = 0 */;
    const vca_api* vca_api_get(int bitDepth)
    {
        if (bitDepth && bitDepth != VCA_DEPTH)
        {
            const char* libname = NULL;
            const char* method = "vca_api_get_" xstr(VCA_BUILD);
            const char* multilibname = "libvca" ext;

            if (bitDepth == 12)
                libname = "libvca_main12" ext;
            else if (bitDepth == 10)
                libname = "libvca_main10" ext;
            else if (bitDepth == 8)
                libname = "libvca_main" ext;
            else
                return NULL;

            const vca_api* api = NULL;
            int reqDepth = 0;

            if (g_recursion > 1)
                return NULL;
            else
                g_recursion++;

#if _WIN32
            HMODULE h = LoadLibraryA(libname);
            if (!h)
            {
                h = LoadLibraryA(multilibname);
                reqDepth = bitDepth;
            }
            if (h)
            {
                api_get_func get = (api_get_func)GetProcAddress(h, method);
                if (get)
                    api = get(reqDepth);
            }
#else
            void* h = dlopen(libname, RTLD_LAZY | RTLD_LOCAL);
            if (!h)
            {
                h = dlopen(multilibname, RTLD_LAZY | RTLD_LOCAL);
                reqDepth = bitDepth;
            }
            if (h)
            {
                api_get_func get = (api_get_func)dlsym(h, method);
                if (get)
                    api = get(reqDepth);
            }
#endif

            g_recursion--;

            if (api && bitDepth != api->bit_depth)
            {
                vca_log(NULL, VCA_LOG_WARNING, "%s does not support requested bitDepth %d\n", libname, bitDepth);
                return NULL;
            }

            return api;
        }

        return &libapi;
    }

    const vca_api* vca_api_query(int bitDepth, int apiVersion, int* err)
    {
        if (apiVersion < 51)
        {
            /* builds before 1.6 had re-ordered public structs */
            if (err) *err = VCA_API_QUERY_ERR_VER_REFUSED;
            return NULL;
        }

        if (err) *err = VCA_API_QUERY_ERR_NONE;

        if (bitDepth && bitDepth != VCA_DEPTH)
        {
            const char* libname = NULL;
            const char* method = "vca_api_query";
            const char* multilibname = "libvca" ext;

            if (bitDepth == 12)
                libname = "libvca_main12" ext;
            else if (bitDepth == 10)
                libname = "libvca_main10" ext;
            else if (bitDepth == 8)
                libname = "libvca_main" ext;
            else
            {
                if (err) *err = VCA_API_QUERY_ERR_LIB_NOT_FOUND;
                return NULL;
            }

            const vca_api* api = NULL;
            int reqDepth = 0;
            int e = VCA_API_QUERY_ERR_LIB_NOT_FOUND;

            if (g_recursion > 1)
            {
                if (err) *err = VCA_API_QUERY_ERR_LIB_NOT_FOUND;
                return NULL;
            }
            else
                g_recursion++;

#if _WIN32
            HMODULE h = LoadLibraryA(libname);
            if (!h)
            {
                h = LoadLibraryA(multilibname);
                reqDepth = bitDepth;
            }
            if (h)
            {
                e = VCA_API_QUERY_ERR_FUNC_NOT_FOUND;
                api_query_func query = (api_query_func)GetProcAddress(h, method);
                if (query)
                    api = query(reqDepth, apiVersion, err);
            }
#else
            void* h = dlopen(libname, RTLD_LAZY | RTLD_LOCAL);
            if (!h)
            {
                h = dlopen(multilibname, RTLD_LAZY | RTLD_LOCAL);
                reqDepth = bitDepth;
            }
            if (h)
            {
                e = VCA_API_QUERY_ERR_FUNC_NOT_FOUND;
                api_query_func query = (api_query_func)dlsym(h, method);
                if (query)
                    api = query(reqDepth, apiVersion, err);
            }
#endif

            g_recursion--;

            if (api && bitDepth != api->bit_depth)
            {
                vca_log(NULL, VCA_LOG_WARNING, "%s does not support requested bitDepth %d\n", libname, bitDepth);
                if (err) *err = VCA_API_QUERY_ERR_WRONG_BITDEPTH;
                return NULL;
            }

            if (err) *err = api ? VCA_API_QUERY_ERR_NONE : e;
            return api;
        }

        return &libapi;
    }
}

