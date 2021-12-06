
#include "common.h"

#include <iostream>
#include <map>

namespace vca {

void vca_log(LogLevel level, std::string error)
{
    static LogLevel appLogLevel     = level;
    static const auto logLevelToInt = std::map<LogLevel, unsigned>(
        {{LogLevel::Debug, 0}, {LogLevel::Info, 1}, {LogLevel::Warning, 2}, {LogLevel::Error, 3}});
    static const auto logLevelName = std::map<LogLevel, std::string>(
        {{LogLevel::Debug, "[Debug] "},
         {LogLevel::Info, "[Info] "},
         {LogLevel::Warning, "[Warning] "},
         {LogLevel::Error, "[Error] "}});

    if (logLevelToInt.at(level) >= logLevelToInt.at(appLogLevel))
        std::cout << logLevelName.at(level) << error << std::endl;
}

} // namespace vca
