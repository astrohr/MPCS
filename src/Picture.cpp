//----------------------------------------------------------

#include "Picture.hpp"

//----------------------------------------------------------



void Picture::approxCoords(float& centerRa, float& centerDec)
{    
    // get the value of m_ra from offset
    m_ra = centerRa + m_offsetRa/3600.f/15.f;
    // make sure that the values make sense
    // eg. -25h is actually 23h
    if (m_ra < 0) m_ra = 24.f - std::fmod(-m_ra, 24);
    else m_ra = std::fmod(m_ra, 24);

    // get the value of m_dec relative to offset
    m_dec = centerDec + m_offsetDec/3600.f;
    // make sure the value is in range 0-360 degrees
    if (m_dec < 0) m_dec = 360.f - std::fmod(-m_dec, 360.f);
    else m_dec = std::fmod(m_dec, 360.f);
    // convert 0-360 degree range to -180 to 180 range
    if (m_dec >= 270.f) m_dec = m_dec-360.f;
    else if (m_dec >= 90.f) m_dec = 180.f-m_dec;
    else;
}