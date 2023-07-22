#include "Log.hpp"

Log::Log(int level = 1)
{
    log_level = level;

    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::string file = std::format("{:04d}-{:02d}-{:02d}.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    output.open(file);
};

Log::~Log()
{
    output.close();
};

void Log::log(std::string &message)
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    message = std::format("[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}] {}\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
    std::cout << message;
    output << message;
};

void Log::setLogLevel(int &level)
{
    if (1 <= level <= 5)
    {
        log_level = level;
    }
    else
    {
        std::string x = "log_level should be set between 1 and 5.";
        err(x);
    }
};

void Log::dbg(std::string &message)
{
    std::string x = "DEBUG:    " + message;
    if (log_level == 1)
    {
        log(x);
    }
};

void Log::msg(std::string &message)
{
    std::string x = "MESSAGE:  " + message;
    if (log_level <= 2)
    {
        log(x);
    }
};

void Log::wrn(std::string &message)
{
    std::string x = "WARNING:  " + message;
    if (log_level <= 4)
    {
        log(x);
    }
};
void Log::err(std::string &message)
{
    std::string x = "ERROR:    " + message;
    if (log_level <= 3)
    {
        log(x);
    }
};
