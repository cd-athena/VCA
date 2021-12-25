
#include "Analyzer.h"

namespace vca {

Analyzer::Analyzer(vca_param cfg)
{
    this->cfg = cfg;
}

void Analyzer::abort()
{
    this->aborted = true;
}

vca_result Analyzer::pushFrame(vca_frame *frame)
{
    vca_frame_results result;
    result.poc        = frame->stats.poc;
    result.complexity = 22;
    this->results.push(result);

    return vca_result::VCA_OK;
}

bool Analyzer::resultAvailable()
{
    if (this->aborted)
        return false;

    std::unique_lock<std::mutex> lock(this->resultsMutex);
    return this->results.empty();
}

std::optional<vca_frame_results> Analyzer::pullResult()
{
    std::unique_lock<std::mutex> lock(this->resultsMutex);
    if (this->results.empty())
        resultsCV.wait(lock, [this]() { return !this->results.empty() || this->aborted; });

    if (this->aborted)
        return {};

    auto res = this->results.front();
    this->results.pop();
    return res;
}

} // namespace vca
