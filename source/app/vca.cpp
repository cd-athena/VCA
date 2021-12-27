/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Vignesh V Menon <vignesh.menon@aau.at>
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

#include <lib/vcaLib.h>

#include "input/Y4MInput.h"
#include "input/YUVInput.h"
#include "vcacli.h"

#include <chrono>
#include <optional>
#include <signal.h>
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

void printStatus(uint32_t frameNum, unsigned framesToBeAnalyzed)
{
    char buf[200];
    static auto startTime      = std::chrono::high_resolution_clock::now();
    static auto prevUpdateTime = std::chrono::high_resolution_clock::now();
    auto now                   = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - prevUpdateTime);
    if (duration < std::chrono::milliseconds(250))
        return;

    prevUpdateTime = now;

    auto elapsedAbs = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    double fps      = elapsedAbs > 0 ? frameNum * 1000. / elapsedAbs : 0;
    if (framesToBeAnalyzed > 0)
    {
        int eta = (int) (elapsedAbs * (framesToBeAnalyzed - frameNum) / ((int64_t) frameNum * 1000));
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

    fprintf(stderr, "%s  \r", buf + 5);
    fflush(stderr); // needed in windows
}
struct CLIOptions
{
    std::string inputFilename;
    bool openAsY4m{};
    unsigned skipFrames{};
    unsigned framesToBeAnalyzed{};
    std::string complexityCSVFilename;
    std::string shotCSVFilename;

    vca_param vcaParam;
};

std::optional<CLIOptions> parseCLIOptions(int argc, char **argv)
{
    bool bError = false;
    CLIOptions options;

    if (argc <= 1)
    {
        vca_log(LogLevel::Error, "No input file. Run vca --help for a list of options.");
        return {};
    }

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
        if (name == "asm")
            options.vcaParam.enableASM = true;
        else if (name == "no-asm")
            options.vcaParam.enableASM = false;
        else if (name == "input")
            options.inputFilename = optarg;
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
        else if (name == "skip")
            options.skipFrames = std::stoul(optarg);
        else if (name == "frames")
            options.framesToBeAnalyzed = std::stoul(optarg);
        else if (name == "complexity-csv")
            options.complexityCSVFilename = optarg;
        else if (name == "shot-csv")
        {
            options.shotCSVFilename           = optarg;
            options.vcaParam.enableShotdetect = true;
        }
        else if (name == "max-thresh")
            options.vcaParam.maxThresh = std::stod(optarg);
        else if (name == "min-thresh")
            options.vcaParam.minThresh = std::stod(optarg);
    }

    if (options.inputFilename.substr(options.inputFilename.size() - 4) == ".y4m")
        options.openAsY4m = true;

    return options;
}

bool checkOptions(CLIOptions options)
{
    if (options.inputFilename.empty())
    {
        vca_log(LogLevel::Error, "No input filename specified");
        return false;
    }

    if (options.vcaParam.frameInfo.bitDepth < 8 || options.vcaParam.frameInfo.bitDepth > 16)
    {
        vca_log(LogLevel::Error, "Bit depth must be between 8 and 16 bits.");
        return false;
    }

    if (!options.openAsY4m
        && (options.vcaParam.frameInfo.width == 0 || options.vcaParam.frameInfo.height == 0))
    {
        vca_log(LogLevel::Error, "No frame size provided.");
        return false;
    }

    return true;
}

void logOptions(CLIOptions options)
{
    vca_log(LogLevel::Info, "Options:   "s);
    vca_log(LogLevel::Info, "  Input file name:   "s + options.inputFilename);
    vca_log(LogLevel::Info, "  Open as Y4m:       "s + (options.openAsY4m ? "True"s : "False"s));
    vca_log(LogLevel::Info, "  Skip frames:       "s + std::to_string(options.skipFrames));
    vca_log(LogLevel::Info, "  Frames to analyze: "s + std::to_string(options.framesToBeAnalyzed));
    vca_log(LogLevel::Info, "  Complexity csv:    "s + options.complexityCSVFilename);
    vca_log(LogLevel::Info, "  Shot csv:          "s + options.shotCSVFilename);
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

int main(int argc, char **argv)
{
#if _WIN32
    char **orgArgv = argv;
    get_argv_utf8(&argc, &argv);
#endif

    vca_log(LogLevel::Debug, "VCA - Video Complexity Analyzer");

    CLIOptions options;
    if (auto cliOptions = parseCLIOptions(argc, argv))
        options = *cliOptions;
    else
    {
        vca_log(LogLevel::Error, "Error parsing parameters");
        return 1;
    }

    logOptions(options);

    if (!checkOptions(options))
    {
        vca_log(LogLevel::Error, "Error checking parameters");
        return 1;
    }

    std::unique_ptr<IInputFile> inputFile;
    if (options.openAsY4m)
        inputFile = std::make_unique<Y4MInput>(options.inputFilename, options.skipFrames);
    else
        inputFile = std::make_unique<YUVInput>(options.inputFilename,
                                               options.vcaParam.frameInfo,
                                               options.skipFrames);

    if (inputFile->isFail())
    {
        vca_log(LogLevel::Error, "Error opening input file");
        return 1;
    }

    options.vcaParam.logFunction = logLibraryMessage;

    auto analyzer = vca_analyzer_open(options.vcaParam);
    if (analyzer == nullptr)
    {
        vca_log(LogLevel::Error, "Error opening analyzer");
        return 2;
    }

    /* Control-C handler */
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
        vca_log(LogLevel::Error,
                "Unable to register CTRL+C handler: " + std::string(strerror(errno)));

    using framePtr = std::unique_ptr<frameWithData>;
    std::queue<framePtr> frameRecycling;
    unsigned poc = 0;
    while (!inputFile->isEof() && !inputFile->isFail()
           && (options.framesToBeAnalyzed == 0 || poc < options.framesToBeAnalyzed))
    {
        framePtr frame;
        if (frameRecycling.empty())
            frame = std::make_unique<frameWithData>();
        else
        {
            frame = std::move(frameRecycling.front());
            frameRecycling.pop();
        }

        if (!inputFile->readFrame(*frame))
        {
            vca_log(LogLevel::Error, "Error reading frame from input");
            return 3;
        }
        frame->vcaFrame.stats.poc = poc;
        vca_log(LogLevel::Debug, "Read frame " + std::to_string(poc) + " from input");

        auto ret = vca_analyzer_push(analyzer, &frame->vcaFrame);
        if (ret == VCA_ERROR)
        {
            vca_log(LogLevel::Error, "Error pushing frame to lib");
            return 3;
        }

        vca_log(LogLevel::Debug, "Pushed frame " + std::to_string(poc) + " to analyzer");

        if (vca_result_available(analyzer))
        {
            vca_frame_results frameResult;
            if (vca_analyzer_pull_frame_result(analyzer, &frameResult) == VCA_ERROR)
            {
                vca_log(LogLevel::Error, "Error pulling frame result");
                return 3;
            }
            // Do something with the result and recycle the frame ...
            vca_log(LogLevel::Debug, "Got results POC " + std::to_string(frameResult.poc) + " averageEnergy " + std::to_string(frameResult.averageEnergy));
        }

        poc++;
    }

    vca_analyzer_close(analyzer);

    return 0;
}
