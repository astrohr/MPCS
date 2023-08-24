//----------------------------------------------------------

#include "Time.hpp"

//----------------------------------------------------------


Time::Time(double JulianDayTime)
: JulianDayTime(JulianDayTime)
{
    auto[year, month, day, hour, minute] = this->JD_to_date(JulianDayTime);
}

Time::Time(int year, int month, int day, int hour, int minute)
: year(year), month(month), day(day), hour(hour), minute(minute)
{
    JulianDayTime = this->date_to_JD(year, month, day, hour, minute);
}

std::tuple<int, int, int, int, int> Time::JD_to_date(double JD)
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

double Time::date_to_JD(int year, int month, int day, int hour, int minute)
{
    // https://en.wikipedia.org/wiki/Julian_day#Converting_Gregorian_calendar_date_to_Julian_Day_Number
    return ((1461*(year+4800+(month-14)/12))/4+(367*(month-2-12*((month-14)/12)))/12-(3*((year+4900+(month-14)/12)/100))/4+day-32075)+(hour-12.0)/24.0+minute/1440.0;
}
