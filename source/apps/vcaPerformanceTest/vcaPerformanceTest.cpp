/*****************************************************************************
 * Copyright (C) 2022 Christian Doppler Laboratory ATHENA
 *
 * Authors: Christian Feldmann <christian.feldmann@bitmovin.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

#include "vcacli.h"

#include <common/input/Y4MInput.h>
#include <common/input/YUVInput.h>
#include <common/stats/YUViewStatsFile.h>
#include <lib/vcaLib.h>

#include <chrono>
#include <optional>
#include <random>
#include <signal.h>
#include <thread>
#include <queue>

#ifdef _WIN32
#include <windows.h>
#pragma warning(disable : 4996)
#endif

using namespace vca;
using namespace std::string_literals;

/* Ctrl-C handler */
static volatile sig_atomic_t b_ctrl_c /* = 0 */;
static void sigint_handler(int)
{
    b_ctrl_c = 1;
}

void logLibraryMessage(void *, LogLevel logLevel, const char *message)
{
    vca_log(logLevel, "[LIB] "s + message);
}

void printStatus(uint32_t frameNum,
                 unsigned framesToBeAnalyzed,
                 bool start        = false,
                 bool printSummary = false)
{
    char buf[200];
    static auto startTime      = std::chrono::high_resolution_clock::now();
    static auto prevUpdateTime = std::chrono::high_resolution_clock::now();
    if (start)
    {
        startTime      = std::chrono::high_resolution_clock::now();
        prevUpdateTime = std::chrono::high_resolution_clock::now();
    }

    auto now = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - prevUpdateTime);
    if (!printSummary && duration < std::chrono::milliseconds(250))
        return;

    prevUpdateTime = now;

    auto elapsedAbsMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime)
                            .count();
    double fps = elapsedAbsMs > 0 ? frameNum * 1000. / elapsedAbsMs : 0;

    if (printSummary)
    {
        fprintf(stdout,
                "vca - Analyzed %d/%d frames, %.2f fps, time %d:%02d:%02d.%03d\n",
                frameNum,
                framesToBeAnalyzed,
                fps,
                int(elapsedAbsMs / 1000 / 3600),
                int((elapsedAbsMs / 1000 / 60) % 60),
                int((elapsedAbsMs / 1000) % 60),
                int(elapsedAbsMs % 1000));
        return;
    }
    else if (framesToBeAnalyzed > 0)
    {
        int eta = (int) (elapsedAbsMs * (framesToBeAnalyzed - frameNum)
                         / ((int64_t) frameNum * 1000));
        sprintf(buf,
                "vca [%.1f%%] %d/%d frames, %.2f fps, eta %d:%02d:%02d",
                100. * frameNum / (framesToBeAnalyzed),
                frameNum,
                framesToBeAnalyzed,
                fps,
                eta / 3600,
                (eta / 60) % 60,
                eta % 60);
    }
    else
        sprintf(buf, "vca %d frames: %.2f fps", frameNum, fps);

    fprintf(stdout, "%s  \r", buf);
    fflush(stdout); // needed in windows
}

struct CLIOptions
{
    unsigned nrFrames{1000};
    vca_param vcaParam;
};

std::optional<CLIOptions> parseCLIOptions(int argc, char **argv)
{
    bool bError = false;
    CLIOptions options;

    int long_options_index = -1;
    while (true)
    {
        auto c = getopt_long(argc, argv, short_options, long_options, &long_options_index);
        if (c == -1)
            break;

        if (c == 'h')
        {
            showHelp();
            return {};
        }

        if (long_options_index < 0 && c > 0)
        {
            for (size_t i = 0; i < sizeof(long_options) / sizeof(long_options[0]); i++)
            {
                if (long_options[i].val == c)
                {
                    long_options_index = (int) i;
                    break;
                }
            }

            if (long_options_index < 0)
            {
                /* getopt_long might have already printed an error message */
                if (c != 63)
                    vca_log(LogLevel::Warning,
                            "internal error: short option " + std::string(1, c)
                                + " has no long option");
                return {};
            }
        }
        if (long_options_index < 0)
        {
            vca_log(LogLevel::Warning, "short option " + std::string(1, c) + " unrecognized");
            return {};
        }

        auto name = std::string(long_options[long_options_index].name);
        auto arg  = std::string(optarg);
        if (name == "iterations")
            options.nrFrames = std::stoul(optarg);
        else if (name == "input-depth")
            options.vcaParam.frameInfo.bitDepth = std::stoul(optarg);
        else if (name == "input-res")
        {
            auto posX = arg.find("x");
            if (posX == std::string::npos)
            {
                vca_log(LogLevel::Error, "Invalid resolution provided. Format WxH.");
                return {};
            }
            options.vcaParam.frameInfo.width  = std::stoul(arg.substr(0, posX));
            options.vcaParam.frameInfo.height = std::stoul(arg.substr(posX + 1));
        }
        else if (name == "input-csp")
        {
            if (arg == "400" || arg == "4:0:0")
                options.vcaParam.frameInfo.colorspace = vca_colorSpace::YUV400;
            else if (arg == "420" || arg == "4:2:0")
                options.vcaParam.frameInfo.colorspace = vca_colorSpace::YUV420;
            else if (arg == "422" || arg == "4:2:2")
                options.vcaParam.frameInfo.colorspace = vca_colorSpace::YUV422;
            else if (arg == "444" || arg == "4:4:4")
                options.vcaParam.frameInfo.colorspace = vca_colorSpace::YUV444;
        }
    }

    return options;
}

bool checkOptions(CLIOptions options)
{
    return true;
}

void logOptions(CLIOptions options)
{
    vca_log(LogLevel::Info, "Options:   "s);
    vca_log(LogLevel::Info, "  Number frames:     "s + std::to_string(options.nrFrames));
}

std::vector<std::unique_ptr<FrameWithData>> generateRandomFrames(vca_frame_info frameInfo,
                                                                 unsigned nrFrames)
{
    if (frameInfo.colorspace != vca_colorSpace::YUV420 || frameInfo.bitDepth != 8)
        throw std::exception("Not implemented yet");

    std::random_device randomDevice;
    std::default_random_engine randomEngine(randomDevice());
    std::uniform_int_distribution<unsigned> uniform_dist(0, 255);

    std::vector<std::unique_ptr<FrameWithData>> frames;
    for (unsigned i = 0; i < nrFrames; i++)
    {
        auto newFrame = std::make_unique<FrameWithData>(frameInfo);
        auto dataSize = newFrame->getFrameSize();
        auto data     = newFrame->getData();
        for (size_t i = 0; i < dataSize; i++)
            data[i] = uint8_t(uniform_dist(randomEngine));
        frames.push_back(std::move(newFrame));
    }
    return std::move(frames);
}

#ifdef _WIN32
/* Copy of x264 code, which allows for Unicode characters in the command line.
 * Retrieve command line arguments as UTF-8. */
static int get_argv_utf8(int *argc_ptr, char ***argv_ptr)
{
    int ret              = 0;
    wchar_t **argv_utf16 = CommandLineToArgvW(GetCommandLineW(), argc_ptr);
    if (argv_utf16)
    {
        int argc   = *argc_ptr;
        int offset = (argc + 1) * sizeof(char *);
        int size   = offset;

        for (int i = 0; i < argc; i++)
            size += WideCharToMultiByte(CP_UTF8, 0, argv_utf16[i], -1, NULL, 0, NULL, NULL);

        char **argv = *argv_ptr = (char **) malloc(size);
        if (argv)
        {
            for (int i = 0; i < argc; i++)
            {
                argv[i] = (char *) argv + offset;
                offset += WideCharToMultiByte(CP_UTF8,
                                              0,
                                              argv_utf16[i],
                                              -1,
                                              argv[i],
                                              size - offset,
                                              NULL,
                                              NULL);
            }
            argv[argc] = NULL;
            ret        = 1;
        }
        LocalFree(argv_utf16);
    }
    return ret;
}
#endif

void runTest(CLIOptions &options, std::vector<std::unique_ptr<FrameWithData>> &pushFrames)
{
    auto analyzer = vca_analyzer_open(options.vcaParam);
    if (analyzer == nullptr)
    {
        vca_log(LogLevel::Error, "Error opening analyzer");
        return;
    }

    printStatus(0, options.nrFrames, true);

    using framePtr = std::unique_ptr<FrameWithData>;
    auto frameIt   = pushFrames.begin();
    for (unsigned poc = 0; poc < options.nrFrames; poc++)
    {
        auto vcaFrame       = (*frameIt)->getFrame();
        vcaFrame->stats.poc = poc;

        vca_log(LogLevel::Debug, "Start push frame " + std::to_string(poc) + " to analyzer");

        auto ret = vca_analyzer_push(analyzer, vcaFrame);
        if (ret == VCA_ERROR)
        {
            vca_log(LogLevel::Error, "Error pushing frame to lib");
            return;
        }

        vca_log(LogLevel::Debug, "Pushed frame " + std::to_string(poc) + " to analyzer");

        if (vca_result_available(analyzer))
        {
            vca_frame_results result;

            vca_log(LogLevel::Debug, "Result available. Pulling it");

            if (vca_analyzer_pull_frame_result(analyzer, &result) == VCA_ERROR)
            {
                vca_log(LogLevel::Error, "Error pulling frame result");
                return;
            }

            vca_log(LogLevel::Debug,
                    "Got results POC " + std::to_string(result.poc) + " averageEnergy "
                        + std::to_string(result.averageEnergy) + " sad "
                        + std::to_string(result.sad));
        }

        printStatus(poc, options.nrFrames);

        frameIt++;
        if (frameIt == pushFrames.end())
            frameIt = pushFrames.begin();
    }

    vca_analyzer_close(analyzer);
    printStatus(options.nrFrames, options.nrFrames, false, true);
}

int main(int argc, char **argv)
{
#if _WIN32
    char **orgArgv = argv;
    get_argv_utf8(&argc, &argv);
#endif

    vca_log(LogLevel::Info, "VCA - Video Complexity Analyzer");

    CLIOptions options;
    if (auto cliOptions = parseCLIOptions(argc, argv))
        options = *cliOptions;
    else
    {
        vca_log(LogLevel::Error, "Error parsing parameters");
        return 1;
    }

    if (options.vcaParam.frameInfo.width == 0 && options.vcaParam.frameInfo.height == 0)
    {
        options.vcaParam.frameInfo.width  = 1920;
        options.vcaParam.frameInfo.height = 1080;
    }

    logOptions(options);

    if (!checkOptions(options))
    {
        vca_log(LogLevel::Error, "Error checking parameters");
        return 1;
    }

    options.vcaParam.logFunction = logLibraryMessage;

    /* Control-C handler */
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        vca_log(LogLevel::Error,
                "Unable to register CTRL+C handler: " + std::string(strerror(errno)));

    auto nrFramesToAllocate = options.vcaParam.nrFrameThreads;
    if (nrFramesToAllocate == 0)
        nrFramesToAllocate = std::thread::hardware_concurrency();

    auto pushFrames = generateRandomFrames(options.vcaParam.frameInfo, nrFramesToAllocate + 1);
    vca_log(LogLevel::Info, "Generated " + std::to_string(pushFrames.size()) + " random frames");

    const std::map<CpuSimd, std::string> cpuSimdNames = {{CpuSimd::None, "None"},
                                                         {CpuSimd::SSE2, "SSE2"},
                                                         {CpuSimd::SSSE3, "SSSE3"},
                                                         {CpuSimd::SSE4, "SSE4"},
                                                         {CpuSimd::AVX2, "AVX2"}};

    for (auto &simd : cpuSimdNames)
    {
        for (unsigned blocksize : {8, 16, 32})
        {
            std::cout << "  [Run test 0 - " << simd.second << " - " << blocksize << "x" << blocksize
                      << "]\n";
            options.vcaParam.cpuSimd   = simd.first;
            options.vcaParam.blockSize = blocksize;

            runTest(options, pushFrames);
            std::cout << "\n";
        }
    }

    return 0;
}
