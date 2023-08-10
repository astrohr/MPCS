//----------------------------------------------------------

#include "Ephemeris.hpp"

//----------------------------------------------------------



Ephemeris::Ephemeris(std::string& raw)
: m_linkVisited(false)
{
    // the raw string should look something like this
    //      -8     -18      <a href="sample_link">Ephemeris #    3</a> MBA soln
    //  offsetRa  offsetDec          link           ephemeris num       type
    // stages:
    //0       1       2             3           4                    5

    raw += ' '; // since we will be checking the character after the current char in the loop, ill add this just in case
    std::string ra, dec, link, num, type;

    int stage = 0;
    for (int i = 0; i < raw.size(); i++){
        if (raw[i] == ' ' && stage != 3) continue;

        if (stage == 0){
            ra += raw[i];
            if (raw[i+1] == ' ') stage++;
        }
        else if (stage == 1){
            dec += raw[i];
            if (raw[i+1] == ' ') stage++;
        }
        else if (stage == 2){
            if (raw[i] == '"') stage++;
        }
        else if (stage == 3){
            link += raw[i];
            if (raw.size() > i+1 && raw[i+1] == '"') stage++;
            // the aditional check just to make sure we dont end up out of bounds if string ends during stage 3
        }
        else if (stage == 4){
            if (isdigit(raw[i])) num += raw[i];
            else if (raw[i] == '>' && raw[i-1] != '"') stage++;
        }
        else if (raw[i] != '\n') type += raw[i];
    }

    try{
        m_offsetRa = std::stoi(ra);
        m_offsetDec = std::stoi(dec);
        m_ephemerisNumber = std::stoi(num);
    }
    catch (std::exception& e) {
        throw utils::ConstructorFail(fmt::format("Bad data provided for ephemeris {}\nprovided: {}", m_ephemerisNumber, raw));
    }

    m_link = link;
    
    // the color is determined by the type
    if (type.empty()) m_color = {0, 255, 0};
    else if (type == "!") m_color = {255, 255, 0};
    else if (type == "!!") m_color = {255, 0, 0};
    else if (type == "***") m_color = {255, 255, 255};
    else if (type == "MBAsoln") m_color = {0, 0, 255};
    else if (type == "JTrojansoln") m_color = {255, 0, 255};
    else{
        fmt::print("Error: object type not found, found only: '{}'\n", type);
        m_color = {255, 255, 255};
    }
}


int Ephemeris::follow_link()
{
    std::vector<std::string> downloaded;

    try{
        utils::get_html(m_link, downloaded, 4500.0);
    }
    catch (utils::DownloadFail& e) {
        fmt::print("Warning: {}\n", e.what());
        return 1;
    }
    catch (utils::ForbiddenLink& e) {
        fmt::print("Warning: {}\n", e.what());
        return 2;
    }

    m_linkVisited = true;

    // get a reference to the first line that contains ephemeris data 
    std::string data;
    for (int i = 0; i < downloaded.size(); i++)
        if (!downloaded[i].empty() && downloaded[i][0] == '2')
            data = downloaded[i];
        
    // remove the text from the end of the string
    for(int i = data.size()-1; i >=0; i--){
        if (isdigit(data[i])){
            data = data.substr(0, i+1);
            break;
        }
    }

    if (data.empty()){
        fmt::print("Warning: No data found on the Ephemeris link");
        return 3;
    }

    // This is the layout of the ephemeris data
    //
    // Date       UT      R.A. (J2000) Decl.  Elong.  V        Motion     Object     Sun         Moon
    //             h m                                      "/min   P.A.  Azi. Alt.  Alt.  Phase Dist. Alt.
    // 2023 06 04 0302   21 42 32.6 +09 16 59 100.9  22.1    0.31  007.1  337  +52   -03    1.00  081  +00  [ommited text]
    
    // that is perfect for stringstreams
    std::stringstream ss_check(data);
    std::string temp;
    // we first check if every element is a number
    while(ss_check >> temp){
        try{
            std::stof(temp);
        }
        catch (std::exception& e) {
            fmt::print("Warning: bad data found for an ephemeris {}: \n\t{}\n", m_ephemerisNumber, data);
            return 3;
        }
    }

    // if all is good, refill the stringstream and analyze it
    std::stringstream ss(data);

    // first time
    int year, month, day, hour_and_min, hour, min;
    year = (ss >> temp) ? std::stoi(temp) : 0;
    month = (ss >> temp) ? std::stoi(temp) : 0;
    day = (ss >> temp) ? std::stoi(temp) : 0;
    hour_and_min = (ss >> temp) ? std::stoi(temp) : 0;
    if (temp.length() == 4){
        hour = hour_and_min / 100;
        min = hour_and_min % 100;
    }
    else{
        hour = hour_and_min;
        min = 0;
    }
    m_JDtime = this->date_to_JD(year, month, day, hour, min);

    // then right ascension
    int ra_whole, ra_min; float ra_sec;
    ra_whole = (ss >> temp) ? std::stoi(temp) : 0;
    ra_min = (ss >> temp) ? std::stoi(temp) : 0;
    ra_sec = (ss >> temp) ? std::stof(temp) : 0.f;
    m_ra = (float)ra_whole + ra_min/60.f + ra_sec/3600.f;

    // then declination
    int dec_whole, dec_min, dec_sec;
    dec_whole = (ss >> temp) ? std::stoi(temp) : 0;
    dec_min = (ss >> temp) ? std::stoi(temp) : 0;
    dec_sec = (ss >> temp) ? std::stoi(temp) : 0;
    m_dec = ((float)abs(dec_whole) + dec_min/60.f + dec_sec/3600.f) * (abs(dec_whole)/dec_whole);

    // then elongation
    m_elong = (ss >> temp) ? std::stof(temp) : 0.f;

    // and magnitude
    m_magnitude = (ss >> temp) ? std::stof(temp) : 0.f;

    // velocity
    m_velocity = (ss >> temp) ? std::stof(temp) : 0.f;

    // angle 
    m_angle = (ss >> temp) ? std::stof(temp) : 0.f;

    // you might think this above is excessive iffing, but i want to avoid the proram crashing if sstream returns ""

    // this is going to be removed soon
    if (data.size() >= 65){
        m_otherData = data.substr(65, data.size()-65);
    }
    return 0;
}

std::tuple<int, int, int, int, int> Ephemeris::JD_to_date(double JD)
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

double Ephemeris::date_to_JD(int year, int month, int day, int hour, int minute)
{
    // https://en.wikipedia.org/wiki/Julian_day#Converting_Gregorian_calendar_date_to_Julian_Day_Number
    return ((1461*(year+4800+(month-14)/12))/4+(367*(month-2-12*((month-14)/12)))/12-(3*((year+4900+(month-14)/12)/100))/4+day-32075)+(hour-12.0)/24.0+minute/1440.0;
}