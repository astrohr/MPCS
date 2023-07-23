//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include "Coordinates.hpp"
#include "Telescope.hpp"

//----------------------------------------------------------

class Observatory{
private:
    
    // observatory ID as on https://minorplanetcenter.net/iau/lists/ObsCodes.html
    std::string ID;

    // the name of the observatory :]
    std::string name;

    // the coordinates of the observatory
    // this is meant to be used if all telescopes are nearby
    Coordinates coords;

    // the list of avaible telescopes
    std::vector<Telescope> telescopes;

public:

    Observatory() = default;
    
    ~Observatory() = default;


    std::string getID() { return ID; }

    std::vector<Telescope>& getTelescopes() { return telescopes; }
    
    void setName(std::string& name) { this->name = name; }

    void setID(std::string& id) { ID = id; }



};
