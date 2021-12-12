#include "vcaLib.h"

DLL_PUBLIC vca_analyzer *vca_analyzer_open(vca_param cfg)
{
    return nullptr;
}

DLL_PUBLIC push_result vca_analyzer_push(vca_analyzer *enc, vca_frame *frame)
{
    return VCA_PUSH_ERROR;
}

DLL_PUBLIC vca_frame_results vca_analyzer_pull_frame_result(vca_analyzer *enc)
{
    vca_frame_results result;
    result.state = VCA_RESULT_ERROR;
    return result;
}

DLL_PUBLIC void vca_analyzer_close(vca_analyzer *enc) {}

DLL_PUBLIC void vca_analyzer_shot_detect(vca_analyzer *enc) {}
