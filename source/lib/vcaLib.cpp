#include "vcaLib.h"

DLL_PUBLIC vca_analyzer *vca_analyzer_open(vca_param cfg)
{
    return nullptr;
}

DLL_PUBLIC push_result vca_analyzer_push(vca_analyzer *enc, vca_picture *pic_in)
{
    return push_result::ERROR;
}

DLL_PUBLIC vca_frame_results vca_analyzer_pull_frame_result(vca_analyzer *enc)
{
    vca_frame_results result;
    result.state = vca_result_state::ERROR;
    return result;
}

DLL_PUBLIC void vca_analyzer_close(vca_analyzer *enc)
{

}

DLL_PUBLIC void vca_analyzer_shot_detect(vca_analyzer *enc)
{

}

