//----------------------------------------------------------

#include "pch.hpp"
#include "utils/utils.hpp"

#include <inipp/inipp.h>

#include "Observatory.hpp"
#include "Object.hpp"

#include "src/cmakevars.h"
#include "window.hpp"

//----------------------------------------------------------

// define global vars

std::string g_mpcsIniPath;
double g_radian = (std::numbers::pi/180.0);



// this function parses the MPCS.ini file
// \param[out] W window width
// \param[out] H window height
// \param[out] FOV telescope FOV
void defaultVariables(unsigned int& W, unsigned int& H, Observatory& obs)
{
    std::ifstream ReadFile(g_mpcsIniPath);

    // initialize inipp
    inipp::Ini<char> ini;
    // inipp magic
    ini.parse(ReadFile);

    // window data
    if (!inipp::get_value(ini.sections["Window"], "W", W)){
        W = 1080;
        fmt::print(
            "Warning: Window width not properly specified\n"
            "Defaulting to W = {}\n", W
        );
    }
    if (!inipp::get_value(ini.sections["Window"], "H", H)){
        H = 920;
        fmt::print(
            "Warning: Window height not properly specified\n"
            "Defaulting to H = {}\n", H
        );
    }

    // observatory data
    std::string id, name; double longitude, latitude;
    if (!inipp::get_value(ini.sections["Observatory"], "CODE", id)){
        ReadFile.close();
        throw mpcsError::InippError("Observatory code not specified\n");
    }
    if (!inipp::get_value(ini.sections["Observatory"], "NAME", name)){
        fmt::print("Warning: Observatory name not specified\n");
        name = "";
    }
    if (!inipp::get_value(ini.sections["Observatory"], "LONGITUDE", longitude)){
        fmt::print("Log: Observatory longitude not found\n");
        longitude = -1000.0;
    }
    if (!inipp::get_value(ini.sections["Observatory"], "LATITUDE", latitude)){
        fmt::print("Log: Observatory latitude not found\n");
        latitude = -1000.0;
    }
    obs.setID(id);
    obs.setName(name);
    obs.setCoords({longitude, latitude});


    // telescope data
    int i = 1; bool shouldScanNext = true;
    while(shouldScanNext)
    {
        int FOV = -1; std::string name = ""; 
        shouldScanNext = false;

        if (inipp::get_value(ini.sections[fmt::format("Telescope{}", i)], "FOV", FOV))
            shouldScanNext = true;

        if (inipp::get_value(ini.sections[fmt::format("Telescope{}", i)], "NAME", name))
            shouldScanNext = true;
        

        if (shouldScanNext){
            if (FOV == -1){
                fmt::print("Warning: FOV for telescope #{} not properly specified\n", i);
                break;
            }
            if (name.empty()){
                fmt::print("Warning: Name for telescope #{} not properly specified\n", i);
                name = "unknown";
            }
        }

        obs.getTelescopes().emplace_back(Telescope(FOV, name));

        i++;
    }

    if (obs.getTelescopes().empty())
        throw mpcsError::InippError("No telescopes (properly) specified");

    std::cout << "\n";
    ReadFile.close();
}

int main(int argc, char **argv)
{
    // -------------------- Init important things
    Observatory observatory;

    std::string version = fmt::format("MPCSolver {}.{}.{}", MPCS_VERSION_MAJOR, MPCS_VERSION_MINOR, MPCS_VERSION_MICRO);
    fmt::println("Log: {}", version);

    // -------------------- find MPCS.ini
    fmt::println("Log: looking for MPCS.ini");
    if (std::filesystem::exists("../resources/MPCS.ini")) g_mpcsIniPath = "../resources/MPCS.ini";
    else if (std::filesystem::exists("./resources/MPCS.ini")) g_mpcsIniPath = "./resources/MPCS.ini";
    else if (std::filesystem::exists("./MPCS.ini")) g_mpcsIniPath = "./MPCS.ini";
    else{
        fmt::print("Error: MPCS.ini does not exist, or isnt in the right directory! (resources)\n\n");
        return 1;
    }
    fmt::println("Log: MPCS.ini found at {}", g_mpcsIniPath);

    // -------------------- read MPCS.ini
    unsigned int W, H;
    try{
        defaultVariables(W, H, observatory);
    } catch (std::exception& e){
        fmt::print("Error in MPCS.ini: {} \n\n", e.what());
        return 1;
    }
    
    // -------------------- get available objects
    std::vector<Object> objects;
    try{
        Object::findObjects(objects);
    } catch (mpcsError::BadData e){
        fmt::print("Bad data error:\n{}", e.what());
        return 1;
    } catch (mpcsError::DownloadFail e){
        fmt::print("Download fail:\n{}", e.what());
        return 1;
    } catch (mpcsError::ForbiddenLink e){
        fmt::print("Forbidden link:\n{}", e.what());
        return 1;
    }

    // -------------------- complete observatory data if it is not completed
    {
        auto [lon, lat] = observatory.getCoords();
        if (
            lon >= 360.0 || lon < 0.0 || 
            lat < -90.0 || lat > 90.0 || 
            observatory.getName().empty()
        ) observatory.fillData();
    }

    // -------------------- start the window
    windowFunction(W, H, objects, observatory);


    return 0;
}
