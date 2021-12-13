#include "vcaLib.h"
#include "analyzer.h"

DLL_PUBLIC vca_analyzer *vca_analyzer_open(vca_param cfg)
{
    return new vca::Analyzer(cfg);
}

DLL_PUBLIC push_result vca_analyzer_push(vca_analyzer *enc, vca_frame *frame)
{
    auto analyzer = (vca::Analyzer*)(enc);
    return analyzer->push(frame);
}

DLL_PUBLIC vca_frame_results vca_analyzer_pull_frame_result(vca_analyzer *enc)
{
    auto analyzer = (vca::Analyzer*)(enc);
    return analyzer->pullResult();
}

DLL_PUBLIC void vca_analyzer_close(vca_analyzer *enc) {}

DLL_PUBLIC void vca_analyzer_shot_detect(vca_analyzer *enc) {}
