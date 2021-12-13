
#include "Analyzer.h"

namespace vca {

Analyzer::Analyzer(vca_param cfg)
{
    this->cfg = cfg;
}

push_result Analyzer::push(vca_frame *frame)
{
    // Todo: Do some real processing on frame
    vca_frame_results result;
    result.state      = VCA_RESULT_OK;
    result.poc        = frame->stats.poc;
    result.complexity = 22;
    this->readyResults.push(result);

    return VCA_PUSH_OK_RESULTS_READY;
}

vca_frame_results Analyzer::pullResult()
{
    if (this->readyResults.empty())
    {
        vca_frame_results result;
        result.state = VCA_RESULT_ERROR;
        return result;
    }
    else
    {
        auto res = this->readyResults.front();
        this->readyResults.pop();
        return res;
    }
}

} // namespace vca
