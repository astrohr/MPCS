//----------------------------------------------------------

#include "structures.hpp"

//----------------------------------------------------------

Coordinates3D skyTo3D(CoordinatesSky& sky)
{ 
    // north is in the direction of the X axis
    // vernal equinox is in the direction of the Z axis

    double decRadians = glm::radians(90.0-sky.dec);
    double raRadians = glm::radians(sky.ra);

    return{
        (float)std::sin(decRadians) * (float)std::cos(raRadians),
        (float)std::sin(decRadians) * (float)std::sin(raRadians),
        (float)std::cos(decRadians)
    };
}