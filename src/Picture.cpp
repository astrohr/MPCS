// Picture.cpp ---------------------------------------------
// this file defines all things declared in Picture.hpp

#include "Picture.hpp"

//----------------------------------------------------------



const std::tuple<float, float> Picture::coords() const { return {m_ra, m_dec}; }
const std::tuple<float, float> Picture::offsets() const { return {m_offsetRa, m_offsetDec}; }
const sf::Text Picture::text() const { return m_text; }
const std::string Picture::sign() const { return (std::string)m_text.getString(); }
const float Picture::percent(int totalEphemeris) const { return (float)m_containedEphemeris/totalEphemeris * 100.f; }
const std::string Picture::percentStr(int totalEphemeris) const {
    float percent = (float)m_containedEphemeris/totalEphemeris * 100.f;
    std::string s = std::to_string(percent);
    s = s.substr(0, s.size()-4) + "%";
    return s;
}

void Picture::approx_coords(float centerRa, float centerDec){
    m_ra = centerRa + (float)m_offsetRa/3600.f/15.f;
    m_dec = centerDec + (float)m_offsetDec/3600.f;
    while(m_ra >= 24.f) m_ra -= 24.f;
    while(m_ra < 0.f) m_ra += 24.f;
    while(m_dec > 90.f || m_dec < -90.f){
        if (m_dec > 90.f) m_dec = 180.f-m_dec;
        if (m_dec < -90.f) m_dec = -180.f-m_dec;
    }
}

void Picture::set_sign(std::string sgn){ m_text.setString(sgn); }

Picture::Picture(float ra, float dec, int objNum) : m_offsetRa(ra), m_offsetDec(dec), m_containedEphemeris(objNum) {
    m_text.setCharacterSize(20);
    m_text.setRotation(180.f);
    m_text.setPosition(m_offsetRa, m_offsetDec);
    m_text.setFillColor(sf::Color(255, 255, 0));
}

