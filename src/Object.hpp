//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

#include "VariantOrbit.hpp"

//----------------------------------------------------------


// class for simplifying the storage of a single object
class Object{
private:
    
    std::string name;

    // Minor planet center score
    int score;

    // time of discovery (unix milis)
    time_t discoveryTime;
    
    // Coordinates
    // note that this is the most probable location
    CoordinatesSky coords;

    // Coordinates in 3D space where the sky sphere is a sphere with a center in 0,0,0
    // this exists to simplify computations
    Coordinates3D coords3d;

    float magnitude;

    // time od last update (unix milis)
    time_t lastUpdate;

    // There is a note section which is usually empty but here it is nevertheless
    std::string note;

    // Number of observations
    int nObs;

    // i dont know what these do
    float arc, H;

    // how many days had it not been seen
    float notSeen;

    // the collection of all variant orbits
    std::vector<VariantOrbit> variantOrbits; 

public:

    Object(
        std::string& name, int& score, time_t& discoveryTime, CoordinatesSky& coords, float& magnitude,
        time_t& lastUpdate, std::string& note, int& nObs, float& arc, float& H, float& notSeen
    );

    ~Object() = default;

    const std::string getName() const { return name; }
    const Coordinates3D& getCoords3D() const { return coords3d; }
    const CoordinatesSky& getCoordsSky()  const { return coords; }
    const float getMagnitude() const { return magnitude; }

    // function that will fill the variantOrbits variable
    // scrapes all available objects from https://cgi.minorplanetcenter.net/cgi-bin/showobsorbs.cgi?Obj=[name]&orb=y
    // \throw DownloadFail, ForbiddenLink, BadData
    void extractOrbits();


    // scrapes all available objects from https://www.minorplanetcenter.net/iau/NEO/neocp.txt
    // \param objects a vector of all objects
    // \throw DownloadFail, ForbiddenLink, BadData
    static void findObjects(std::vector<Object>& objects);
};
