// Picture.cpp ---------------------------------------------
// this file defines all things declared in Picture.hpp

#include "Picture.hpp"

//----------------------------------------------------------



void Picture::approx_coords(float& centerRa, float& centerDec){
    // Add the values together
    m_ra = centerRa + (float)m_offsetRa/3600.f/15.f;
    m_dec = centerDec + (float)m_offsetDec/3600.f;

    // Make sure that the values of right ascension are possible
    while(m_ra >= 24.f) m_ra -= 24.f;
    while(m_ra < 0.f) m_ra += 24.f;

    // Make sure that te values of declination are posible
    while(m_dec > 90.f || m_dec < -90.f){
        if (m_dec > 90.f) m_dec = 180.f-m_dec;
        if (m_dec < -90.f) m_dec = -180.f-m_dec;
    }
}
