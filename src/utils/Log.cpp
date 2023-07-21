#include "Log.hpp"

void Log::log(std::string message)
{

    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    message = std::format("[{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}] {}\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, message);
    std::cout << message;
    std::ofstream log_file;
    std::string file = std::format("{:04d}-{:02d}-{:02d}.log", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    if (first)
    {
        log_file.open(file);
        first = false;
    }
    else
    {
        log_file.open(file, std::ofstream::app);
    }

    log_file << message;
    log_file.close();
};

void Log::setLogLevel(int level)
{
    if (1 <= level <= 5)
    {
        log_level = level;
    }
    else
    {
        err("log_level should be set between 1 and 5.");
    }
};

void Log::dbg(std::string message)
{
    if (log_level == 1)
    {
        log("DEBUG:    " + message);
    }
};

void Log::msg(std::string message)
{
    if (log_level <= 2)
    {
        log("MESSAGE:  " + message);
    }
};

void Log::err(std::string message)
{
    if (log_level <= 3)
    {
        log("WARNING:  " + message);
    }
};

void Log::wrn(std::string message)
{
    if (log_level <= 4)
    {
        log("ERROR:    " + message);
    }
};

int main()
{
    Log logger;
    logger.dbg("debug");
    logger.msg("dsgdfgfgdfgdfg");
    logger.wrn("opzorilo");
    logger.err("x");

    return 0;
}