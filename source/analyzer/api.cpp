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

#include "analyzer.h"

#if EXPORT_C_API
/* these functions are exported as C functions (default) */
using namespace VCA_NS;
extern "C" {
#else
/* these functions exist within private namespace (multilib) */
namespace X265_NS {
#endif

    vca_analyzer* vca_analyzer_open(vca_param *p)
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

        Analyzer* analyzer = NULL;
        vca_param* param = param_alloc();
        if (!param)
            goto fail;

        memcpy(param, p, sizeof(vca_param));

        vca_setup_primitives(param);

        if (check_params(param))
            goto fail;

        analyzer = new Analyzer;

        // may change params for auto-detect, etc
        analyzer->configure(param);

        analyzer->create();

        /* Try to open complexity CSV file handle */
        if (analyzer->m_param->csv_E_h_fn)
        {
            analyzer->m_param->csv_E_h_fpt = vca_csv_E_h_log_open(analyzer->m_param);
            if (!analyzer->m_param->csv_E_h_fpt)
            {
                vca_log(analyzer->m_param, VCA_LOG_ERROR, "Unable to open complexity CSV log file <%s>, aborting\n", analyzer->m_param->csv_E_h_fn);
                analyzer->m_aborted = true;
            }
        }
        /* Try to open shot detection CSV file handle */
        if (analyzer->m_param->csv_shot_fn)
        {
            analyzer->m_param->csv_shot_fpt = vca_csv_shot_log_open(analyzer->m_param);
            if (!analyzer->m_param->csv_shot_fpt)
            {
                vca_log(analyzer->m_param, VCA_LOG_ERROR, "Unable to open shot detection CSV log file <%s>, aborting\n", analyzer->m_param->csv_shot_fn);
                analyzer->m_aborted = true;
            }
        }
        analyzer->m_param = param;
        memcpy(analyzer->m_param, param, sizeof(vca_param));
        if (analyzer->m_aborted)
            goto fail;

        print_params(param);
        return analyzer;

    fail:
        delete analyzer;
        param_free(param);
        return NULL;
    }

    void vca_analyzer_parameters(vca_analyzer *enc, vca_param *out)
    {
        if (enc && out)
        {
            Analyzer *analyzer = static_cast<Analyzer*>(enc);
            memcpy(out, analyzer->m_param, sizeof(vca_param));
        }
    }

    int vca_analyzer_analyze(vca_analyzer *enc, vca_picture *pic_in)
    {
        if (!enc)
            return -1;

        if (!pic_in)
            return 0;

        Analyzer *analyzer = static_cast<Analyzer*>(enc);
        int numAnalyzed;

        do
        {
            numAnalyzed = analyzer->analyze(pic_in);
            if (numAnalyzed)
                vca_csv_E_h_log_frame(analyzer->m_param, pic_in);
        } while (numAnalyzed == 0 && !pic_in);

        if (numAnalyzed < 0)
            analyzer->m_aborted = true;

        return numAnalyzed;
    }

    void vca_analyzer_close(vca_analyzer *enc)
    {
        if (enc)
        {
            Analyzer *analyzer = static_cast<Analyzer*>(enc);
            analyzer->destroy();
            delete analyzer;
        }
    }

    void vca_analyzer_shot_detect(vca_analyzer *enc)
    {
        Analyzer *analyzer = static_cast<Analyzer*>(enc);
        int numFrames = analyzer->m_param->totalFrames;

        /* First pass */
        analyzer->isNewShot[0] = VCA_NEW_SHOT;
        analyzer->isNewShot[1] = VCA_NOT_NEW_SHOT;
        analyzer->prevShotPos = 0;
        int not_sure_count = 0;
        for (int i = 2, j = 0; i < numFrames; i++)
        {
            if (analyzer->epsilons[i] > analyzer->m_param->maxThresh)
            {
                analyzer->isNewShot[i] = VCA_NEW_SHOT;
                analyzer->prevShotPos = i;
            }
            else if (analyzer->epsilons[i] < analyzer->m_param->minThresh)
            {
                analyzer->isNewShot[i] = VCA_NOT_NEW_SHOT;
            }
            else
            {
                analyzer->isNewShot[i] = VCA_NOTSURE_NEW_SHOT;
                analyzer->isNotSureShot[j] = i;
                analyzer->prevShotDist[j] = i - analyzer->prevShotPos;
                not_sure_count++;
                j++;
            }
        }
        /* If there are no VCA_NOTSURE_NEW_SHOT entries, shot detection has been completed */
        vca_log(analyzer->m_param, VCA_LOG_DEBUG, "First pass of shot detection complete!\n");
        if (!not_sure_count)
            return;


        /* Second pass */
        for (int j = 0; j < not_sure_count; j++)
        {
            int fps_value = analyzer->m_param->fpsNum / analyzer->m_param->fpsDenom;
            if (analyzer->prevShotDist[j] > fps_value && analyzer->prevShotDist[j + 1] > fps_value)
            {
                analyzer->isNewShot[analyzer->isNotSureShot[j]] = VCA_NEW_SHOT;
            }
            else
            {
                analyzer->isNewShot[analyzer->isNotSureShot[j]] = VCA_NOT_NEW_SHOT;
            }
        }
        vca_log(analyzer->m_param, VCA_LOG_DEBUG, "Second pass of shot detection complete!\n");
    }

    void vca_analyzer_shot_print(vca_analyzer *enc)
    {
        Analyzer *analyzer = static_cast<Analyzer*>(enc);
        int numFrames = analyzer->m_param->totalFrames;
        int shot_count = 1;
        for (int i = 0; i < numFrames; i++)
        {
            if (analyzer->isNewShot[i] == VCA_NEW_SHOT)
            {
                if (analyzer->m_param->csv_shot_fpt)
                {
                    if (i)
                        fprintf(analyzer->m_param->csv_shot_fpt, "%d,\n", i - 1);
                    fprintf(analyzer->m_param->csv_shot_fpt, "%d, %d,", shot_count, i);
                    shot_count++;
                    fflush(stderr);
                }
                vca_log(analyzer->m_param, VCA_LOG_INFO, "IDR at POC %d\n", i);
            }
            if (analyzer->m_param->csv_shot_fpt)
            {
                if (i == (numFrames - 1))
                    fprintf(analyzer->m_param->csv_shot_fpt, "%d,\n", numFrames - 1);
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

        &vca_analyzer_open,
        &vca_analyzer_parameters,
        &vca_analyzer_analyze,
        &vca_analyzer_close,
        &vca_analyzer_shot_detect,
        &vca_analyzer_shot_print,
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

