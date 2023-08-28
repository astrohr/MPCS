//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

class VariantOrbit{
private:
    
    float H, G;

    std::string epoch;

    double M;

    double perihelion, node, inclination;

    // orbit ellipse parameters
    double e, n, a;

    int nOpp;

    // arc in days
    int arc;

    float rms;

    // vartiant orbit ID as provided by minor planet center
    int ID;

    // list of orbits (sorted by time)
    std::list<Orbit> orbits;

public:

    VariantOrbit(
        float& H, float& G, std::string& epoch, double& M, double& perihelion, double& node, double& inclination,
        double& e, double& n, double& a, int& nOpp, int& arc, float& rms, int& ID
    )
    : H(H), G(G), epoch(epoch), M(M), perihelion(perihelion), node(node), inclination(inclination),
      e(e), n(n), a(a), nOpp(nOpp), arc(arc), rms(rms), ID(ID) 
    {}
    
    ~VariantOrbit() = default;

};
