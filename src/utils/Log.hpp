//----------------------------------------------------------

#pragma once

#include "../pch.hpp"
#include <ctime>
#include <format>

//----------------------------------------------------------

// Logger class

class Log
{
private:
    int log_level = 1; // 1: all, 2: mute debug, 3: mute message, 4: mute error, 5: mute all
    bool first = true;
    void log(std::string message);

public:
    int getLogLevel() { return log_level; };
    void setLogLevel(int level);

    void dbg(std::string message);
    void msg(std::string message);
    void err(std::string message);
    void wrn(std::string message);
};
