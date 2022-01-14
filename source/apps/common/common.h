#pragma once

#include <lib/vcaLib.h>

#include <string>
#include <vector>

#pragma once

namespace vca {

class FrameWithData
{
public:
    FrameWithData() = delete;
    FrameWithData(const vca_frame_info &frameInfo);
    ~FrameWithData() = default;

    uint8_t *getData() const
    {
        return (uint8_t *) (this->data.data());
    }
    size_t getFrameSize() const
    {
        return this->data.size();
    }
    vca_frame *getFrame()
    {
        return &this->vcaFrame;
    }

private:
    std::vector<uint8_t> data;
    vca_frame vcaFrame;
};

void vca_log(LogLevel level, std::string error);

} // namespace vca