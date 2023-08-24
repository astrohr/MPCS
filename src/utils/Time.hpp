//----------------------------------------------------------

#pragma once

#include "pch.hpp"

//----------------------------------------------------------


// class made for easier manipulation and storage of time values
class Time{
private:

    // the time of the ephemeris in julian day
    // https://en.wikipedia.org/wiki/Julian_day
    double JulianDayTime;

    int year, month, day, hour, minute;

public:

    // constructor that uses julian day
    Time(double JulianDayTime);

    // constructor that uses actual time
    Time(int year, int month, int day, int hour, int minute);

    // get juilan day time
    double getJD() { return JulianDayTime; }

    // get year, month, day, hour, minute
    std::tuple<int, int, int, int, int> getTime() { return {year, month, day, hour, minute}; }


    // function for converting dates to JD time
    // \param JD Julian day time
    // \returns tuple with year, month, day, hour and minute
    static std::tuple<int, int, int, int, int> JD_to_date(double JD);

    // function for converting JD time to dates
    // \returns Julian day time
    static double date_to_JD(int year, int month, int day, int hour, int minute);

};


// map that links strings to their months
std::map<std::string, int> monthVals = 
{
    {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4},
    {"May", 5}, {"Jun", 6}, {"Jul", 7}, {"Aug", 8},
    {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12},
};