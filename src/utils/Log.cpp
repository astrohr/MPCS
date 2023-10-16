#include "Log.hpp"

Log::Log(int level) : log_level(level)
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::string file = fmt::format("{:04d}-{:02d}-{:02d}.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    output.open(file);
};

Log::~Log()
{
    output.close();
};

void Log::setLogLevel(const int &level)
{
    if ( level >= 1 && level <= 5)
    {
        log_level = level;
    }
    else
    {
        std::string x = "log_level should be set between 1 and 5.";
        err(x);
    }
};

void Log::log(std::string &message, bool &hide)
{
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    message = fmt::format("{}\n", message);
    std::string log_message = fmt::format("[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}] ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec) + message;
    output << log_message;
    if (!hide)
    {
        std::cout << message;
    }
};

void Log::dbg(const std::string &message, bool hide /*=false*/)
{
    if (log_level == 1)
    {
        std::string x = "DEBUG:    " + message;
        log(x, hide);
    }
};

void Log::msg(const std::string &message, bool hide /*=false*/)
{
    if (log_level <= 2)
    {
        std::string x = "MESSAGE:  " + message;
        log(x, hide);
    }
};

void Log::wrn(const std::string &message, bool hide /*=false*/)
{
    if (log_level <= 3)
    {
        std::string x = "WARNING:  " + message;
        log(x, hide);
    }
};

void Log::err(const std::string &message, bool hide /*=false*/)
{
    if (log_level <= 4)
    {
        std::string x = "ERROR:    " + message;
        log(x, hide);
    }
};

Log logs;
