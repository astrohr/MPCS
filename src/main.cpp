//----------------------------------------------------------

#include "pch.hpp"

#include <inipp/inipp.h>

#include "Observatory.hpp"
#include "Object.hpp"
#include "utils/utils.hpp"

#include "src/cmakevars.h"
#include "window.hpp"

//----------------------------------------------------------

// scrapes all available objects from https://www.minorplanetcenter.net/iau/NEO/neocp.txt
// \param objects a vector of all objects
// \throw DownloadFail, ForbiddenLink, BadData
void findObjects(std::vector<Object>& objects)
{
    std::string objectsLink = "https://www.minorplanetcenter.net/iau/NEO/neocp.txt";
    std::vector<std::string> raw;
    get_html(objectsLink, raw);

    // a single string looks like this
    // P21Inbw 100 2023 08 24.5   2.3376 +30.8741 19.8 Updated Aug. 24.62 UT            4   0.03 22.1  0.050
    //  name  score discovery          coords      mag     last update        note     nObs  arc   H   days not seen

    // scan every line and make an object :D
    for(auto& line : raw){
        int lastUpdatePosition = 48; // which character is the last position location
        int notePosition = 70; // which character is the note location
        int nobsPosition = 78; // which character is the nObs location

        if (!line.size()) continue;
        else if (line.size() < 100) 
            throw mpcsError::BadData(std::format("Bad data found on {}: Less than 100 chars per line", objectsLink));

        std::stringstream ss1(line.substr(0, lastUpdatePosition)), ss2(line.substr(nobsPosition));
        std::string temp;

        std::string name;
        int score, year, month; float day, mag; double ra, dec;
        int nObs; float arc, H, dns;

        try{
            ss1 >> name;

            ss1 >> temp; score = std::stoi(temp);
            ss1 >> temp; year = std::stoi(temp);
            ss1 >> temp; month = std::stoi(temp);
            ss1 >> temp; day = std::stof(temp);
            ss1 >> temp; ra = std::stod(temp);
            ss1 >> temp; dec = std::stod(temp);
            ss1 >> temp; mag = std::stof(temp);

            ss2 >> temp; nObs = std::stoi(temp);
            ss2 >> temp; arc = std::stof(temp);
            ss2 >> temp; H = std::stof(temp);
            ss2 >> temp; dns = std::stof(temp);
        } catch(std::exception& e){
            throw mpcsError::BadData(std::format("Bad data found on {}: \n{}", objectsLink, e.what()));
        }

        // instead of trowing an error here (and in other places), write the data to a log and continue scanning
    	
        std::string lastUpdateStr = line.substr(lastUpdatePosition, notePosition-lastUpdatePosition);
        std::string note = line.substr(notePosition, nobsPosition-notePosition);

        // make the time variable
        float minute = (day - (float)((int)day)) * 60.f;
        float second = (minute - (float)((int)minute)) * 60.f;
        Time discoveryTime(year, month, (int)day, (int)minute, (int)second);

        // make a coordinates variable
        Coordinates coords = {ra, dec};

        // turn the lastUpdate string into Time
        float time;
        int year2=-1, month2; // year is -1 because year isnt provided and its not needed to specify it in this case
        float day2, hour2, sec2;
        try{          
            ss1 >> temp; ss1 >> temp; 
            if (temp.back() == '.') temp.pop_back(); // remove the dot from the end of the month
            month2 = monthVals[temp];    
            ss1 >> temp;
            day2 = std::stof(temp);
        } catch(std::exception& e){
            throw mpcsError::BadData(std::format("Bad data found on {}: \n{}", objectsLink, e.what()));
        }
        float minute2 = (day2 - (float)((int)day2)) * 60.f;
        float second2 = (minute2 - (float)((int)minute2)) * 60.f;
        Time lastUpdate(year2, month2, (int)day2, (int)minute2, (int)second2);

        // check if the note has content
        bool noteContent = false;
        for(auto& ch : note){
            if (ch != ' '){
                noteContent = true;
                break;
            }
        }
        if(!noteContent) note.clear(); // if not, clear it

        // create the object (can throw an error)
        objects.emplace_back(Object(name, score, discoveryTime, coords, mag, lastUpdate, note, nObs, arc, H, dns));
    }
}

// this function parses the MPCS.ini file
// \param[out] W window width
// \param[out] H window height
// \param[out] FOV telescope FOV
void defaultVariables(unsigned int& W, unsigned int& H, Observatory& obs)
{
    std::ifstream ReadFile("../resources/MPCS.ini");
    if (!ReadFile.is_open())
    {
        ReadFile.open("./resources/MPCS.ini");
        if (!ReadFile.is_open()){
            throw mpcsError::InippError("MPCS.ini does not exist, or isnt in the right directory! (resources)\n\n");
        }
    }

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
    std::string id, name;
    if (!inipp::get_value(ini.sections["Observatory"], "CODE", id)){
        ReadFile.close();
        throw mpcsError::InippError("Observatory code not specified\n");
    }
    if (!inipp::get_value(ini.sections["Observatory"], "NAME", name)){
        fmt::print("Warning: Observatory name not specified\n");
        name = "";
    }
    obs.setID(id);
    obs.setName(name);


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
        findObjects(objects);
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


    windowFunction(W, H, "Minor Planet Center Solver 3");

    return 0;
}
