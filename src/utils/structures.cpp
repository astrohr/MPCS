//----------------------------------------------------------

#include "structures.hpp"

//----------------------------------------------------------

Coordinates3D skyTo3D(CoordinatesSky& sky)
{ 
    // north is in the direction of the X axis
    // vernal equinox is in the direction of the Z axis

    float decRadians = glm::radians(90.0-sky.dec);
    float raRadians = glm::radians(sky.ra);

    return{
        std::sin(decRadians) * std::cos(raRadians),
        std::sin(decRadians) * std::sin(raRadians),
        std::cos(decRadians)
    };
}