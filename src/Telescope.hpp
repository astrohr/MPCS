//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include "Coordinates.hpp"

//----------------------------------------------------------

class Telescope{
private:
    
    // telescope ID as on https://minorplanetcenter.net/iau/lists/ObsCodes.html
    std::string ID;

    std::string name;

    // the coordinates of the telescope
    Coordinates coords;

    // telescope field of view
    float FOV;

    // the locations where specific pictures will be taken
    std::vector<Coordinates> pictures;

public:

    Telescope();
    
    ~Telescope();

};
