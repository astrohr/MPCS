#pragma once

#include <SFML/Graphics/Color.hpp>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <tuple>

#include "net.hpp"

class Ephemeris{
private:
    float m_ra, m_dec;
    int m_offsetRa, m_offsetDec;
    int m_ephemerisNumber;
    std::string m_elong, m_velocity, m_angle, m_magnitude; // these are strings cuz i dont need to calculate anything, duh 
    std::string m_time;
    std::string m_otherData; // this is just the rest of the context data
    std::string m_link;
    sf::Color m_color;
public:
    //context is a temporary variable that gives an example of what an ephemeris looks like
    std::string m_context;
    const std::tuple<float, float> coords() const;
    const std::tuple<float, float> offsets() const;
    const std::string mag() const;
    const std::string time() const;
    const std::string context() const;
    const sf::Color color() const;

    void approx_coords(float centerRa, float centerDec);

    int follow_link();

    //the constructor is passed the string that describes the ephemeris in the source and parses trough it
    Ephemeris(std::string raw);

};