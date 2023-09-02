//----------------------------------------------------------

#pragma once

#include "pch.hpp"

//----------------------------------------------------------

// Sky position storage struct
struct CoordinatesSky{
    // Right ascension
    double ra;

    // Declination
    double dec;

};


// Sky position storage struct but for azimuth and altitude
struct CoordinatesSkyLocal{
    // Azimuth
    double az;

    // Altitude
    double alt;
    
};


// Geographical coordinates storage struct
struct CoordinatesGeo{
    // Longitude
    double lon;

    // Latitude
    double lat;
    
};


// Coordinates of a 3D system
struct Coordinates3D{
    // axis position
    float X, Y, Z;
    
};


// coordinate conversion functions

Coordinates3D skyTo3D(CoordinatesSky& sky);



// NEO orbit types
enum OrbitType{
    LUNAR_DISTANCE, // distance < 0.0027 AU (MPC signature: ***)
    VERY_CLOSE, // LUNAR_DISTANCE < distance < 0.01 AU (MPC signature: !!) 
    CLOSE, // VERY_CLOSE < distance < 0.05 AU (MPC signature: !)
    STANDARD, // CLOSE < distance < MAIN_BELT
    MAIN_BELT, // object is in the main belt (MPC signature: MBAsoln)
    JUPITER_TROJAN // object is a jupiter trojan (MPC signature: JTrojansoln)
};


// struct for storing all orbit data for a single point in time
struct Orbit{
    // sky location
    CoordinatesSky coords;

    // local sky location
    CoordinatesSkyLocal coordsLoc;

    // magnitude
    float mag;

    // angle of movement
    float angle;

    // angular velocity
    float velocity;

    // the type of the orbit
    OrbitType type;
    
    // time at which orbit is relevant (unix milis)
    size_t time;
};