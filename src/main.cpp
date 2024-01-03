//----------------------------------------------------------

#include "pch.hpp"
#include "utils/utils.hpp"
#include "src/cmakevars.h"

#include <inipp/inipp.h>

#include "Observatory.hpp"
#include "Object.hpp"

#include "window.hpp"

//----------------------------------------------------------

// define global vars (from pch.hpp)

std::string g_resourcesPath;

// function checks if a given path has all resources that this program requires
// \param[in] path path to the folder to check
bool checkPath(const std::string& path)
{
    fmt::println("Log: Checking path {}", path);
    bool allresources = true;
    if (!std::filesystem::exists(path)){
        fmt::println("Log: Folder {} doesnt exist", path);    
        return false;
    }

    fmt::println("Log: resources folder found at {}", path);
    fmt::println("Log: Checking resources...");

    if (std::filesystem::exists(path+"/MPCS.ini")) fmt::println("Log: MPCS.ini found");
    else{
        fmt::println("Log: MPCS.ini not found");
        allresources = false;
    }

    if (std::filesystem::exists(path+"/shaders/fragment.glsl")) fmt::println("Log: shaders/fragment.glsl found");
    else{
        fmt::println("Log: shaders/fragment.glsl not found");
        allresources = false;
    }

    if (std::filesystem::exists(path+"/shaders/vertex.glsl")) fmt::println("Log: shaders/vertex.glsl found");
    else{
        fmt::println("Log: shaders/vertex.glsl not found");
        allresources = false;
    }

    if (allresources) fmt::println("Log: resources found at {}", path);
    return allresources;
}

// this function parses the MPCS.ini file
// \param[out] W window width
// \param[out] H window height
// \param[out] FOV telescope FOV
void defaultVariables(unsigned int& W, unsigned int& H, Observatory& obs)
{
    std::ifstream ReadFile(g_resourcesPath+"/MPCS.ini");

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
    std::string id, name; float longitude, latitude;
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
    unsigned int W, H; // window dimensions

    // -------------------- Log the state of the program
    fmt::println("Info: MPCSolver version {}.{}.{}", MPCS_VERSION_MAJOR, MPCS_VERSION_MINOR, MPCS_VERSION_MICRO);
    fmt::println("Info: GLFW version {}.{}.{}", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);
    fmt::println("Info: GLEW version {}.{}.{}", GLEW_VERSION_MAJOR, GLEW_VERSION_MINOR, GLEW_VERSION_MICRO);
    fmt::println("Info: GLM version {}.{}.{}.{}", GLM_VERSION_MAJOR, GLM_VERSION_MINOR, GLM_VERSION_PATCH, GLM_VERSION_REVISION);

    // -------------------- find MPCS.ini
    fmt::println("Log: looking for the resources folder...");
    if (checkPath("../resources")) g_resourcesPath = "../resources";
    else if (checkPath("./resources")) g_resourcesPath = "./resources";
    else if (checkPath("./MPCS.ini")) g_resourcesPath = ".";
    else{
        fmt::print("Error: resources not found!\n\n");
        return 1;
    }

    // -------------------- read MPCS.ini
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
    fmt::println("Log: Found {} objects", objects.size());

    // -------------------- complete observatory data if it is not completed
    {
        auto [lon, lat] = observatory.getCoords();
        if (
            lon >= 360.f || lon < 0.f || 
            lat < -90.f || lat > 90.f || 
            observatory.getName().empty()
        ) observatory.fillData();
    }

    // -------------------- start the window
    windowFunction(W, H, objects, observatory);


    return 0;
}
