#include "vcaLib.h"
#include "analyzer/analyzer.h"

#include "analyzer/Analyzer.h"

DLL_PUBLIC vca_analyzer *vca_analyzer_open(vca_param param)
{
    return new vca::Analyzer(param);
}

DLL_PUBLIC vca_result vca_analyzer_push(vca_analyzer *enc, vca_frame *frame)
{
    if (enc == nullptr)
        return vca_result::VCA_ERROR;

    auto analyzer = (vca::Analyzer *) enc;
    if (analyzer == nullptr)
        return vca_result::VCA_ERROR;

    return analyzer->pushFrame(frame);
}

DLL_PUBLIC bool vca_result_available(vca_analyzer *enc)
{
    auto analyzer = (vca::Analyzer *) (enc);
    return analyzer->resultAvailable();
}

DLL_PUBLIC vca_result vca_analyzer_pull_frame_result(vca_analyzer *enc, vca_frame_results *result)
{
    if (enc == nullptr || result == nullptr)
        return vca_result::VCA_ERROR;

    auto analyzer = (vca::Analyzer *) (enc);
    if (analyzer == nullptr)
        return vca_result::VCA_ERROR;

    auto pulledResult = analyzer->pullResult();
    if (!pulledResult)
        return vca_result::VCA_ERROR;

    *result = *pulledResult;
    return vca_result::VCA_OK;
}

DLL_PUBLIC void vca_analyzer_close(vca_analyzer *enc)
{
    auto analyzer = (vca::Analyzer *) enc;
    delete analyzer;
}

DLL_PUBLIC void vca_analyzer_shot_detect(vca_analyzer *enc) {}
