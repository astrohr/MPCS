//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include "Coordinates.hpp"

//----------------------------------------------------------

class Telescope{
private:

    // what is the telescope called?
    std::string name;

    // the coordinates of the telescope
    Coordinates coords;

    // telescope field of view (in arcseconds)
    float FOV;

    // the rotation of the telescope field of view with regards to the horizon line (in radians)
    double rotation;

    // the locations where specific pictures will be taken
    std::vector<Coordinates> pictures;

public:

    Telescope() = default;

    Telescope(float fov, std::string name)
    : FOV(fov), name(name) {}

    ~Telescope() = default;

};