#include "Logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

std::ofstream Logger::s_LogFile;
std::mutex Logger::s_Mutex;

void Logger::init(const std::string& filename) {
    s_LogFile.open(filename);
}

void Logger::shutdown()
{
    if (s_LogFile.is_open())
        s_LogFile.close();
}

void Logger::write(LogLevel level, const std::string& message)
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    std::stringstream ss;
    ss << "[" << getTimestamp() << "]"
       << "[" << levelToString(level) << "] "
       << message;

    std::string finalMsg = ss.str();

    std::cout << finalMsg << std::endl;

    if (s_LogFile.is_open())
        s_LogFile << finalMsg << std::endl;

}

std::string Logger::getTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    std::stringstream ss;
    ss << std::put_time(&tm, "%H:%M:%S");

    return ss.str();
}

std::string Logger::levelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Debug: return "DEBUG";
    }
    return "";
}

void Logger::printStackTrace()
{
#ifdef _WIN32
    void* stack[64];
    unsigned short frames = CaptureStackBackTrace(0, 64, stack, nullptr);

    std::cout << "Stack Trace:" << std::endl;

    for (unsigned int i = 0; i < frames; i++)
    {
        std::cout << "  " << stack[i] << std::endl;
    }
#else
    std::cout << "StackTrace not implemented." << std::endl;
#endif
}

