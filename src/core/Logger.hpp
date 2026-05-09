#ifndef COZYGAME_LOGGER_H
#define COZYGAME_LOGGER_H

#include <string>
#include <fstream>
#include <mutex>
#include <sstream>

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
    static void log(LogLevel level, const std::string& format, Args&&... args) {
        std::string message = formatMessage(format, std::forward<Args>(args)...);
        write(level, message);
    }

private:
    template<typename T>
    static std::string toString(T&& value) {
        std::ostringstream stream;
        stream << std::forward<T>(value);
        return stream.str();
    }

    static void appendFormatted(std::string& output, const std::string& format);

    template<typename T, typename... Args>
    static void appendFormatted(std::string& output, const std::string& format, T&& value, Args&&... args) {
        const std::size_t placeholder = format.find("{}");
        if (placeholder == std::string::npos) {
            output += format;
            return;
        }

        output.append(format, 0, placeholder);
        output += toString(std::forward<T>(value));
        appendFormatted(output, format.substr(placeholder + 2), std::forward<Args>(args)...);
    }

    template<typename... Args>
    static std::string formatMessage(const std::string& format, Args&&... args) {
        std::string message;
        message.reserve(format.size() + sizeof...(Args) * 8);
        appendFormatted(message, format, std::forward<Args>(args)...);
        return message;
    }

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
