//----------------------------------------------------------

#pragma once

//----------------------------------------------------------

// Sky position storage struct
struct Coordinates{
    // Right ascension
    double ra;

    // Declination
    double dec;

};


// Sky position storage struct but for azimuth and altitude
struct CoordinatesLocal{
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
