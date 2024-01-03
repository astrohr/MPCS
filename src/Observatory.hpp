//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

#include <inipp/inipp.h>

#include "Telescope.hpp"

//----------------------------------------------------------

class Observatory{
private:
    
    // observatory ID as on https://minorplanetcenter.net/iau/lists/ObsCodes.html
    std::string ID;

    // the name of the observatory :]
    std::string name;

    // coordinates are in the WGS-84 coordinate system
    // because of this they will not perfectly match with the regular geographic coordinates (google maps), however 
    // due to the irregular shape of the planet, these coordinates will give a more precise image of the night sky
    // this is why those coordinates should not be displayed anywhere since they will cause confusion
    CoordinatesGeo coords;

    // the list of avaible telescopes
    std::vector<Telescope> telescopes;

public:

    Observatory() = default;
    
    ~Observatory() = default;


    std::string getID() const { return ID; }

    std::vector<Telescope>& getTelescopes() { return telescopes; }
    
    CoordinatesGeo& getCoords() { return coords; }

    std::string getName() const { return name; }

    void setCoords(CoordinatesGeo coords) { this->coords = coords; }

    void setName(std::string& name) { this->name = name; }

    void setID(std::string& id) { ID = id; }


    // the function that will pull the observatory data from https://minorplanetcenter.net/iau/lists/ObsCodes.html
    // \throw DownloadFail, ForbiddenLink, BadData
    // \return true if failed, false if it didnt
    bool fillData();

};
