//----------------------------------------------------------

#include "pch.hpp"

#include <inipp/inipp.h>
#include <args.hxx>

#include "Observatory.hpp"
#include "Telescope.hpp"

#include "src/cmakevars.h"
#include "window.hpp"

//----------------------------------------------------------


// this function parses the MPCS.ini file
// \param[out] W window width 
// \param[out] H window height 
// \param[out] FOV telescope FOV
void defaultVariables(unsigned int& W, unsigned int& H, Observatory& obs)
{
    std::ifstream ReadFile("../resources/MPCS.ini");
    if (!ReadFile.is_open()){
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
    try{ // try catch because we are parsing for an integer
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
    } catch (std::exception e){
        ReadFile.close();
        throw e;
    }

    // observatory data
    std::string id, name;
    if (inipp::get_value(ini.sections["Observatory"], "CODE", id)){
        ReadFile.close();
        throw mpcsError::InippError("Observatory code not specified\n");
    }
    if (inipp::get_value(ini.sections["Observatory"], "NAME", name)){
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

        try{ // try catch because we are parsing for an integer
            if (!inipp::get_value(ini.sections[fmt::format("Telescope{}", i)], "FOV", FOV))
                shouldScanNext = true;
        } catch (std::exception e){
            ReadFile.close();
            throw e;
        }

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

    fmt::print("\n");
    ReadFile.close();
}


// args syntax: ./MPCS [-u|--url <str>] [-e|--exposition <int>] [-n|--number <int>] [-c|--copy] [-x|--exit] [-f|--fov <int>]
int main(int argc, char** argv)
{
    // -------------------- Init important things
    Observatory observatory;

    ObjectDatabase database;
    Camera cam;
    
    std::string obj_url = "";
    int pic_exposition=0, pic_number=0;
    bool to_clipboard = false, close_after = false;

    std::string version = fmt::format("MPCSolver {}.{}.{}", MPCS_VERSION_MAJOR, MPCS_VERSION_MINOR, MPCS_VERSION_MICRO);

    // -------------------- Argument parser
    args::ArgumentParser parser("MPCS - Minor Planet Center Solver", version + '\n');

    // arguments
    args::HelpFlag help(parser, "help", "Display this message", {'h', "help"});
    args::ValueFlag<std::string> url(parser, "url", "the url to the object offsets link", {'u', "url"});
    args::ValueFlag<int> exposition(parser, "exposition", "the exposition duration (seconds)", {'e', "expositon"});
    args::ValueFlag<int> number(parser, "number", "number of pictures to be taken", {'n', "number"});
    args::Flag copy(parser, "copy", "copy to clipboard", {'c', "copy"});
    args::Flag exit(parser, "exit", "exit the program after use", {'x', "exit"});

    // try parsing arguments
    try{
        parser.ParseCLI(argc, argv);
        fmt::print("{}\n", version);
    } 
    // if parsing fails, print help message and inform user of the error
    catch (args::Help) {
        std::cout << std::endl << parser << std::endl;
        return 0;
    }
    catch (args::ParseError e) {
        fmt::print("\nError: {}\n\n", e.what());
        std::cout << parser << std::endl;
        return 1;
    }
    catch (args::ValidationError e){
        fmt::print("\nError: {}\n\n", e.what());
        std::cout << parser << std::endl;
        return 1;
    }

    // if parsing was successful store the inputed arguments
    if (url) obj_url = args::get(url);
    if (exposition) pic_exposition = args::get(exposition);
    if (number) pic_number = args::get(number);
    if (exit) close_after = true; 
    // if no parameters were passed, assume to_clipboard to be true
    if (copy || (!url && !exposition && !number && !exit)) to_clipboard = true; 


    // -------------------- read MPCS.ini
    try{
        unsigned int W, H;
        defaultVariables(W, H, observatory);
        cam.setDimensions(W, H);
    }
    catch (std::exception& e){
        fmt::print("Error in MPCS.ini: {} \n\n", e.what());
        return 1;
    }

    // -------------------- main loop
    while(true)
    {
        if (obj_url == "") fmt::print("Insert the website URL:\n");
        //this while loop is here to make sure a link is provided
        while(!obj_url.size()) std::getline(std::cin, obj_url);
        fmt::print("\n");
        
        int greska = database.fill_database(obj_url);
        if (greska){
            bool retry = false;
            if (greska == 1){
                fmt::print("Link interaction failed, retry? (y/n): ");
                std::string s; std::cin >> s;
                if (!s.empty() && (s[0] == 'y' || s[0] == 'Y')) retry = true;
            }

            if (close_after && !retry) break;
            else{
                if (!retry) obj_url = "";
                pic_exposition = pic_number = 0;
                database.reset();
                fmt::print("\n\n");
                continue;
            }
        }

        cam.reset_position(database.getFOV(), database);
        
        fmt::print("\nObject: {}\n", database.name());

        if (!pic_number){
            fmt::print("Insert the ammount of pictures: ");
            std::cin >> pic_number;
        }
        database.set_amount(pic_number);

        if (!pic_exposition){
            fmt::print("Insert the exposure length (in seconds): ");
            std::cin >> pic_exposition;
        }
        database.set_exposure(pic_exposition);

        windowFunction(database, cam);

        database.export_observation_targets(to_clipboard);
        
        if (close_after){
            if (!to_clipboard){
                fmt::print("Press enter to exit");
                std::cin.ignore();
            }
            break;
        }
        else{
            pic_exposition = pic_number = 0;
            obj_url = "";
            database.reset();
            fmt::print("\n\n");
        }
    }

    return 0;
}
