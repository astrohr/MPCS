//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include "utils/utils.hpp"
#include "Orbit.hpp"

//----------------------------------------------------------

class VariantOrbit{
private:
    
    // vartiant orbit ID as provided by minor planet center
    int ID;

    // list of orbits (sorted by time)
    std::list<Orbit> orbits;

public:

    VariantOrbit();
    
    ~VariantOrbit();

};
