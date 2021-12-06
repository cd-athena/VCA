/// This is the public interface for the bitstream lib

#pragma once

#include "vcaColorSpace.h"
#include <stdint.h>


#if defined(_MSC_VER) && !defined(VCA_STATIC_BUILD)
#define DLL_PUBLIC __declspec(dllexport)
#else
#define DLL_PUBLIC __attribute__((__visibility__("default")))
#endif

extern "C" {

/* vca_analyzer:
 *      opaque handler for analyzer */
typedef void vca_analyzer;

/* vca_picyuv:
 *      opaque handler for PicYuv */
typedef struct vca_picyuv vca_picyuv;

struct vca_frame_texture_t
{
    int32_t *m_ctuAbsoluteEnergy;
    double *m_ctuRelativeEnergy;
    int32_t m_variance;
    int32_t m_avgEnergy;
};

/* Frame level statistics */
struct vca_frame_stats
{
    int poc;
    int32_t e_value;
    double h_value;
    double epsilon;
};

enum class vca_result_state
{
    OK,
    ERROR,
    PUSH_MORE_FRAMES,
    DONE
};

struct vca_frame_results
{
    vca_result_state state;

    // Todo - redo this
    int poc;
    int complexity;
};

struct vca_frame_info
{
    unsigned width{};
    unsigned height{};
    unsigned bitDepth{8};
    vca_colorSpace colorspace{vca_colorSpace::YUV420};
};

/* Used to pass pictures into the analyzer, and to get picture data back out of
 * the analyzer.  The input and output semantics are different */
struct vca_frame
{
    /* Must be specified on input pictures, the number of planes is determined
     * by the colorSpace value */
    void *planes[3]{nullptr, nullptr, nullptr};

    /* Stride is the number of bytes between row starts */
    int stride[3]{0, 0, 0};

    vca_frame_stats stats;
    vca_frame_info info;
};

/* vca input parameters
 *
 */
struct vca_param
{
    bool enableShotdetect{}; /* Enable shot detection algorithm using epsilon feature */

    bool enableASM{true};

    /*== Internal Picture Specification ==*/
    vca_frame_info frameInfo{};

    double minThresh{}; /* Minimum threshold for epsilon in shot detection */
    double maxThresh{}; /* Maximum threshold for epsilon in shot detection */
};

/* Create a new analyzer or nullptr if the config is invalid.
 */
DLL_PUBLIC vca_analyzer *vca_analyzer_open(vca_param cfg);

/* Push a frame to the analyzer and start the analysis
 */
enum class push_result
{
    OK,
    ERROR,
    PULL_RESULTS_FIRST
};
DLL_PUBLIC push_result vca_analyzer_push(vca_analyzer *enc, vca_frame *pic_in);

DLL_PUBLIC vca_frame_results vca_analyzer_pull_frame_result(vca_analyzer *enc);

DLL_PUBLIC void vca_analyzer_close(vca_analyzer *enc);
DLL_PUBLIC void vca_analyzer_shot_detect(vca_analyzer *enc);

} // extern "C"
