//----------------------------------------------------------

#pragma once

#include "pch.hpp"

//----------------------------------------------------------


// function for converting dates to JD time
// \param JD Julian day time
// \returns tuple with year, month, day, hour and minute
std::tuple<int, int, int, int, int> JD_to_date(double JD);

// function for converting JD time to dates
// \returns Julian day time
double date_to_JD(int year, int month, int day, int hour, int minute);

// function for converting dates to unix time
// \returns unix timestamp
time_t date_to_unix_milis(int year, int month, int day, int hour, int minute, int second);

// map that links strings to their months
extern std::map<std::string, int> monthVals;