#ifndef COZYGAME_LOGGER_H
#define COZYGAME_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <fmt/core.h>

enum class LogLevel {
    Info,
    Warning,
    Error,
    Debug
};

class Logger {
public:
    static void init(const std::string& filename = "log.txt");
    static void shutdown();

    template<typename... Args>
    static void log(LogLevel level, fmt::format_string<Args...> fmtStr, Args&&... args) {
        std::string message = fmt::format(fmtStr, std::forward<Args>(args)...);
        write(level, message);
    }

private:
    static void write(LogLevel level, const std::string& message);
    static std::string getTimestamp();
    static std::string levelToString(LogLevel level);
    static void printStackTrace();

    static std::ofstream s_LogFile;
    static std::mutex s_Mutex;

};

#define LOG_INFO(...)  Logger::log(LogLevel::Info, __VA_ARGS__)
#define LOG_WARN(...)  Logger::log(LogLevel::Warning, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(LogLevel::Error, __VA_ARGS__)
#define LOG_DEBUG(...) Logger::log(LogLevel::Debug, __VA_ARGS__)

#endif //COZYGAME_LOGGER_H