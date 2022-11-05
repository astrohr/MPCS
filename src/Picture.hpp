#pragma once
// Picture.hpp ---------------------------------------------
// this file is used for easier storage and manipulation of picture data
// * in this program, the "picture" is a field where the telescope will be taking final images

#include <SFML/Graphics/Text.hpp>

#include "pch.hpp"
// Precompiled headers this file uses:
// tuple

//----------------------------------------------------------



class Picture{
private:
    float m_ra, m_dec;
    float m_offsetRa, m_offsetDec;
    int m_containedEphemeris;
    sf::Text m_text;
public:
    const std::tuple<float, float> coords() const;
    const std::tuple<float, float> offsets() const;
    const sf::Text text() const;
    const std::string sign() const;
    const float percent(int totalEphemeris) const;
    const std::string percentStr(int totalEphemeris) const;

    void approx_coords(float centerRa, float centerDec);

    void set_sign(std::string sgn);

    Picture(float ra, float dec, int objNum);

};
