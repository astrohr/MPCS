//----------------------------------------------------------

#pragma once

#include "pch.hpp"
//----------------------------------------------------------

// Logger class

class Log
{
private:
    int log_level; // 1: all, 2: mute debug, 3: mute message, 4: mute error, 5: mute all
    void log(std::string &message, bool &hide);
    std::ofstream output;

public:
    Log(int level);
    ~Log();
    int getLogLevel() { return log_level; };
    void setLogLevel(const int &level);

    void dbg(const std::string &message, bool hide = false);
    void msg(const std::string &message, bool hide = false);
    void wrn(const std::string &message, bool hide = false);
    void err(const std::string &message, bool hide = false);
};
