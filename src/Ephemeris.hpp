#pragma once
// Ephemeris.hpp -------------------------------------------
// this header defines the ephemeris class meant for easier Ephemeris data manipulation

#include "pch.hpp"
// Precompiled headers used in this file:
// sstream
// vector, string, tuple

#include "net.hpp"

//----------------------------------------------------------



// Class for simplifying the storage and manipulation of Ephemeris datapoints
class Ephemeris{
private:
    
    // Ephemeris right ascension in hours
    float m_ra;
    // Ephemeris declination in degrees
    float m_dec;

    // Offset in arcseconds from the middle 0,0 coordinate
    float m_offsetRa, m_offsetDec;

    // the number of the ephemeris object (its ID of sorts) 
    int m_ephemerisNumber;

    // elongation of the ephemeris object
    float m_elong;

    // magnitude of the ephemeris object
    float m_magnitude;

    // velocity of the ephemeris object
    float m_velocity;

    // angle of the velocity
    float m_angle;

    // the time of the ephemeris in julian day
    // https://en.wikipedia.org/wiki/Julian_day
    double m_JDtime;

    // this is just the rest of the context data, i will remove it soon
    std::string m_otherData;

    // link to the data about ephemeris
    std::string m_link;
    // has link been visited?
    bool m_linkVisited;

    // the color of the ephemeris
    std::tuple<int, int, int> m_color;

public:

    // Ephemeris constructor
    // \param raw string containing data about the ephemeris from the object offsets link
    Ephemeris(std::string& raw);



    // Coordinates getter
    // \returns tuple with coordinates
    const std::tuple<float, float> coords() const
    { return {m_ra, m_dec}; }

    // Offsets getter
    // \returns tuple with offsets
    const std::tuple<float, float> offsets() const
    { return {m_offsetRa, m_offsetDec}; }

    // Magnitude getter
    // \returns string with magnitude data
    const float mag() const
    { return m_magnitude; }

    // Time getter
    // \returns JD time of the ephemeris
    const double timeJD() const
    { return m_JDtime; }

    // Context getter
    // \returns string with time data
    const std::string context() const
    { return fmt::format("{:5.1f} {:5.1f} {:7.2f}  {:05.1f}{}", m_elong, m_magnitude, m_velocity, m_angle, m_otherData); }

    // Color getter
    // \returns the RGB color tuple of the ephemeris
    const std::tuple<int, int, int> color() const
    { return m_color; }

    // function that tells has ephemeris link beeen visited
    // \returns true if yes, false if no, what did you think would happen huh
    const bool linkVisited() const
    { return m_linkVisited; }


    // This function follows the link that is provided by the constructor and pulls the data from it
    // for now it just looks at the first ephemeris, but that will be changed
    int follow_link();
    

    
    // function for converting dates to JD time
    // \param JD Julian day time
    // \returns tuple with year, month, day, hour and minute
    static std::tuple<int, int, int, int, int> JD_to_date(double JD);

    // function for converting JD time to dates
    // \returns Julian day time
    static double date_to_JD(int year, int month, int day, int hour, int minute);

};