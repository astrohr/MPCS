//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include "utils/utils.hpp"
#include "VariantOrbit.hpp"

//----------------------------------------------------------

class Object{
private:
    
    std::string name;

    // the collection of all variant orbits
    std::vector<VariantOrbit> variantOrbits; 

public:

    Object();
    
    ~Object();


};
