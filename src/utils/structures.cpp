//----------------------------------------------------------

#pragma once

#include "structures.hpp"

//----------------------------------------------------------

Coordinates3D SkyTo3D(CoordinatesSky& sky)
{
    double decRadians = (90.0-sky.dec) * g_radian;
    double raRadians = sky.ra * g_radian;

    return{
        std::sin(decRadians) * std::cos(raRadians),
        std::sin(decRadians) * std::sin(raRadians),
        std::cos(decRadians)
    };
}