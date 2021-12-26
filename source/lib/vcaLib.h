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

enum class LogLevel
{
    Error,
    Warning,
    Info,
    Debug
};

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

struct vca_frame_results
{
    /* Pointer to storage for one value per block in the frame.
     * The caller must make sure that this is pointing to a valid and big enough block of memory.
     */
    int32_t *energyPerBlock{};
    int32_t eValue{};

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
    uint8_t *planes[3]{nullptr, nullptr, nullptr};

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

    vca_frame_info frameInfo{};

    // Size (width/height) of the analysis block. Must be 8, 16 or 32.
    unsigned blockSize{32};

    double minThresh{}; /* Minimum threshold for epsilon in shot detection */
    double maxThresh{}; /* Maximum threshold for epsilon in shot detection */

    unsigned nrFrameThreads{1};
    unsigned nrSliceThreads{1};

    // Logging
    void (*logFunction)(void *, LogLevel, const char *){};
    void *logFunctionPrivateData{};
};

/* Create a new analyzer or nullptr if the config is invalid.
 */
DLL_PUBLIC vca_analyzer *vca_analyzer_open(vca_param cfg);

/* Push a frame to the analyzer and start the analysis.
 * Note that only the pointers will be copied but no ownership of the memory is
 * transferred to the library. The caller must make sure that the pointers are
 * valid until the frame was analyzed. Once a results for a frame was pulled the
 * library will not use pointers anymore.
 */
typedef enum
{
    VCA_OK = 0,
    VCA_ERROR
} vca_result;

DLL_PUBLIC vca_result vca_analyzer_push(vca_analyzer *enc, vca_frame *pic_in);

/* Check if a result is available to pull.
 */
DLL_PUBLIC bool vca_result_available(vca_analyzer *enc);

/* Pull a result from the analyzer. This may block until a result is available.
 * Use vca_result_available if you want to only check if a result is ready.
 */
DLL_PUBLIC vca_result vca_analyzer_pull_frame_result(vca_analyzer *enc, vca_frame_results *result);

DLL_PUBLIC void vca_analyzer_close(vca_analyzer *enc);
DLL_PUBLIC void vca_analyzer_shot_detect(vca_analyzer *enc);

} // extern "C"
