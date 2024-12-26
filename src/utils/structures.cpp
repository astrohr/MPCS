//----------------------------------------------------------

#include "structures.hpp"

//----------------------------------------------------------

glm::vec3 skyTo3D(CoordinatesSky& sky)
{ 
    // north is in the direction of the Z axis from 0,0
    // vernal equinox is in the direction of the X axis

    float decRadians = glm::radians(90.0-sky.dec);
    float raRadians = glm::radians(sky.ra);

    return glm::vec3(
        std::sin(decRadians) * std::cos(raRadians),
        std::sin(decRadians) * std::sin(raRadians),
        std::cos(decRadians)
    );
}