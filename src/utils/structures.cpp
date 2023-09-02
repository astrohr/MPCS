//----------------------------------------------------------

#pragma once

#include "structures.hpp"

//----------------------------------------------------------

Coordinates3D skyTo3D(CoordinatesSky& sky)
{ 
    // north is in the direction of the X axis
    // vernal equinox is in the direction of the Z axis

    double decRadians = (90.0-sky.dec) * g_radian;
    double raRadians = sky.ra * g_radian;

    return{
        std::sin(decRadians) * std::cos(raRadians),
        std::sin(decRadians) * std::sin(raRadians),
        std::cos(decRadians)
    };
}