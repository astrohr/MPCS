//----------------------------------------------------------

#pragma once

#include "utils/Time.hpp"
#include "Coordinates.hpp"
#include "OrbitType.hpp"

//----------------------------------------------------------

// struct for storing all orbit data for a single point in time
struct Orbit{
    // sky location
    Coordinates coords;

    // local sky location
    CoordinatesLocal coordsLoc;

    // magnitude
    float mag;

    // angle of movement
    float angle;

    // angular velocity
    float velocity;

    // the type of the orbit
    OrbitType type;
    
    // time at which orbit is relevant
    Time time
};