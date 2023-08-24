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

    // Time of discovery
    Time discoveryTime;
    
    // Coordinates
    // note that this is the most probable location
    Coordinates coords;

    float magnitude;

    // Time od last update
    Time lastUpdate;

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
        std::string& name, int& score, Time& discoveryTime, Coordinates& coords, float& magnitude,
        Time& lastUpdate, std::string& note, int& nObs, float& arc, float& H, float& notSeen
    )
    : name(name), score(score), discoveryTime(discoveryTime), coords(coords), magnitude(magnitude),
      lastUpdate(lastUpdate), note(note), nObs(nObs), arc(arc), H(H), notSeen(notSeen)
    {}

    ~Object();

};
