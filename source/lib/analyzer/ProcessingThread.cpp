#include "ProcessingThread.h"

#include "DCTTransforms.h"
#include "WeightedCoeffSum.h"
#include "common.h"

namespace {

void copyPixelValuesToBuffer(unsigned blockOffsetLuma,
                             unsigned blockSize,
                             unsigned bitDepth,
                             uint8_t *src,
                             unsigned srcStride,
                             int16_t *buffer)
{
    if (bitDepth == 8)
    {
        src += blockOffsetLuma;
        for (unsigned y = 0; y < blockSize; y++, src += srcStride)
            for (unsigned x = 0; x < blockSize; x++)
                *(buffer++) = int16_t(src[x]);
    }
    else
    {
        auto input = (int16_t *) (src) + blockOffsetLuma;
        for (unsigned y = 0; y < blockSize; y++, input += srcStride, buffer += blockSize)
            std::memcpy(buffer, input, blockSize * sizeof(int16_t));
    }
}

void performDCT(unsigned blockSize, int16_t *pixelBuffer, int16_t *coeffBuffer)
{
    // DCT
    switch (blockSize)
    {
        case 32:
            vca::dct32_c(pixelBuffer, coeffBuffer, 32);
            break;
        case 16:
            vca::dct16_c(pixelBuffer, coeffBuffer, 16);
            break;
        case 8:
            vca::dct8_c(pixelBuffer, coeffBuffer, 8);
            break;
        default:
            throw std::invalid_argument("Invalid block size " + std::to_string(blockSize));
    }
}

} // namespace

namespace vca {

bool ProcessingThread::computeWeightedDCTEnergy(Job &job, Result &result)
{
    const auto frame = job.frame;
    if (frame == nullptr)
    {
        log(this->cfg, LogLevel::Error, "Invalid frame pointer");
        return false;
    }

    const auto blockSize = this->cfg.blockSize;
    const auto bitDepth  = frame->info.bitDepth;

    if (frame->info.bitDepth > 8)
    {
        // What do we do for 16 bit? Could we just scle down to 8?
        log(this->cfg, LogLevel::Error, "16 bit not implemented yet");
        return false;
    }

    auto src       = frame->planes[0];
    auto srcStride = frame->stride[0];

    auto [widthInBlocks, heightInBlock] = getFrameSizeInBlocks(this->cfg, frame->info);
    auto totalNumberBlocks              = widthInBlocks * heightInBlock;
    auto widthInPixels                  = widthInBlocks * blockSize;
    auto heightInPixels                 = heightInBlock * blockSize;

    if (result.energyPerBlock.size() < totalNumberBlocks)
        result.energyPerBlock.resize(totalNumberBlocks);

    // First, we will copy the source to a temporary buffer which has one int16_t value
    // per sample.
    //   - This may only be needed for 8 bit values. For 16 bit values we could also
    //     perform this directly from the source buffer. However, we should check the
    //     performance of that approach (i.e. the buffer may not be aligned)

    auto blockIndex      = 0u;
    int32_t frameTexture = 0;
    for (unsigned blockX = 0; blockX < widthInPixels; blockX += blockSize)
    {
        for (unsigned blockY = 0; blockY < heightInPixels; blockY += blockSize)
        {
            auto blockOffsetLuma = blockX + (blockY * srcStride);
            copyPixelValuesToBuffer(blockOffsetLuma,
                                    blockSize,
                                    bitDepth,
                                    src,
                                    srcStride,
                                    this->pixelBuffer);

            performDCT(blockSize, this->pixelBuffer, this->coeffBuffer);

            result.energyPerBlock[blockIndex] = calculateWeightedCoeffSum(blockSize,
                                                                          this->coeffBuffer);
            frameTexture += result.energyPerBlock[blockIndex];

            blockIndex++;
        }
    }

    result.averageEnergy = frameTexture / totalNumberBlocks;

    return true;
}

ProcessingThread::ProcessingThread(vca_param cfg,
                                   MultiThreadQueue<Job> &jobs,
                                   MultiThreadQueue<Result> &results,
                                   unsigned id)
{
    this->cfg = cfg;
    this->id  = id;

    this->thread = std::thread(&ProcessingThread::threadFunction,
                               this,
                               std::ref(jobs),
                               std::ref(results));
}

void ProcessingThread::threadFunction(MultiThreadQueue<Job> &jobQueue,
                                      MultiThreadQueue<Result> &results)
{
    while (!this->aborted)
    {
        auto job = jobQueue.pop();
        if (!job)
            continue;

        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(this->id) + ": Start work on job " + job->infoString());

        Result result;
        result.poc = job->frame->stats.poc;

        computeWeightedDCTEnergy(*job, result);

        log(this->cfg,
            LogLevel::Debug,
            "Thread " + std::to_string(this->id) + ": Finished work on job " + job->infoString());

        results.push(result);
    }

    log(this->cfg, LogLevel::Info, "Thread " + std::to_string(this->id) + " quit");
}

void ProcessingThread::abort()
{
    this->aborted = true;
}
void ProcessingThread::join()
{
    this->aborted = true;
    this->thread.join();
}

} // namespace vca
