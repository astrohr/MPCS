//----------------------------------------------------------

#pragma once

#include "pch.hpp"

//----------------------------------------------------------



// Class for easier storage of the location of a picture area and its properties
class Picture{
private:

    // Picture right ascension in hours
    float m_ra;
    // Picture declination in degrees
    float m_dec;
    // ^ These variables are filled in the approx_coords function

    // Offset in arcseconds from the middle 0,0 coordinate
    float m_offsetRa, m_offsetDec;

    // The ammount of Ephemeris objects that are within the picture area
    int m_containedEphemeris;

    // The text that will be displayed with the picture (its name)
    std::string m_name;

public:

    // The Picture constructor
    // \param ra Right ascension of the picture (in hours)
    // \param dec Declination of the picture (in degrees)
    // \param objNum The ammount of Ephemeris objects within the picture area
    Picture(float ra, float dec, int objNum)
    : m_offsetRa(ra), m_offsetDec(dec), m_containedEphemeris(objNum) {}



    // Offsets getter
    // \returns tuple with right ascencion in hours and declination (in arcseconds)
    const std::tuple<float, float> getOffsets() const
    { return {m_offsetRa, m_offsetDec}; }

    // Coordinates getter
    // \returns tuple with right ascencion and declination offsets (in arcseconds)
    const std::tuple<float, float> getCoords() const
    { return {m_ra, m_dec}; }

    // Name getter
    // \returns the name of the picture
    const std::string getName() const
    { return m_name; }
    
    // Name setter
    // \param name the string with the name of the picture object
    void setSign(std::string name)
    { m_name = name; }
    
    // Contained Ephemeris getter
    // \returns the ammount of contained ephemeris
    const int getContainedEphemeris() const 
    { return m_containedEphemeris; }



    // This function calculates the absolute coordinates of the picture area
    // The coordinates are stored in private m_ra and m_dec variables
    // \param centerRa right ascension of 0,0 (in hours)
    // \param centerDec declination of 0,0 (in degrees)
    void approxCoords(float& centerRa, float& centerDec);

};
