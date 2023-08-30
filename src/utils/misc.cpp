//----------------------------------------------------------

#include "misc.hpp"

//----------------------------------------------------------

std::tuple<int, int, int, int, int> JD_to_date(double JD)
{
    // https://en.wikipedia.org/wiki/Julian_day#Julian_or_Gregorian_calendar_from_Julian_day_number
    int var1 = 4*((int)(JD+0.5) + 1401 + (((4*(int)(JD+0.5)+274277)/146097)*3)/4 - 38) + 3;
    int var2 = 5*(var1%1461/4) + 2;

    int day = (var2%153)/5 + 1;
    int month = (var2/153+2)%12 + 1;
    int year = var1/1461 - 4716 + (14-month)/12;

    double temp, remainder = modf(JD, &temp);
    if (remainder > 0.5) remainder -= 0.5;
    else remainder += 0.5;
    int secs = (int)(remainder * 86400);
    int hour = secs / 3600;
    int min = secs % 3600 / 60;

    return {year, month, day, hour, min};
}

double date_to_JD(int year, int month, int day, int hour, int minute)
{
    // https://en.wikipedia.org/wiki/Julian_day#Converting_Gregorian_calendar_date_to_Julian_Day_Number
    return ((1461*(year+4800+(month-14)/12))/4+(367*(month-2-12*((month-14)/12)))/12-(3*((year+4900+(month-14)/12)/100))/4+day-32075)+(hour-12.0)/24.0+minute/1440.0;
}

time_t date_to_unix_milis(int year, int month, int day, int hour, int minute, int second)
{
    std::tm time_in = {0};
    time_in.tm_year = year - 1900; // years since 1900
    time_in.tm_mon = month - 1;   // months since January - [0,11]
    time_in.tm_mday = day;
    time_in.tm_hour = hour;
    time_in.tm_min = minute;
    time_in.tm_sec = second;

    // Convert the tm structure to time_t and then to the system_clock::time_point
    time_t time_temp = std::mktime(&time_in);
    if (time_temp == -1) throw std::runtime_error("Error: Failed to convert to time_t.");

    auto tp = std::chrono::system_clock::from_time_t(time_temp);
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

std::map<std::string, int> monthVals = 
{
    {"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4},
    {"May", 5}, {"Jun", 6}, {"Jul", 7}, {"Aug", 8},
    {"Sep", 9}, {"Oct", 10}, {"Nov", 11}, {"Dec", 12},
};