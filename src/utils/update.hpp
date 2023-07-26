#include "pch.hpp"


void run(std::string latest_ver);
bool update(std::string latest_ver);

bool check_ver(std::string version)
{

    std::cout << "Checking for updates...\n" << std::endl;
    std::string short_ver = version.substr(10);
    cpr::Response r = cpr::Get(cpr::Url{"https://github.com/astrohr/MPCS/releases/latest"});

    std::string latest_ver = r.url.str();
    latest_ver = latest_ver.substr(latest_ver.find_last_of('/') + 2);
    if (r.status_code == 0)
    {
        std::cerr << r.error.message << std::endl;
        return false;
    }
    else if (r.status_code >= 400)
    {
        std::cerr << "Error [" << r.status_code << "] making request" << std::endl;
        return false;
    }
    else
    {
        std::cout << "Latest MPCS version is: " << latest_ver << std::endl;
        if (short_ver == latest_ver)
        {
            std::cout << "MPCS up-to-date!\n\n"
                      << std::endl;
            return false;
        }
        else
        {
            if (short_ver.rfind("2.", 0) == 0)
            {
                std::cout << "MPCS 2.x.x is no longer supported!\n";
            }
            std::cout << "Your's is " << short_ver << " - Would you like to update? [Y/n]: ";
            char choice;
            std::cin >> choice;
            if (choice == 'Y')
            {
                std::cout << std::endl;
                return update(latest_ver);
            }
            else
            {
                std::cout << "\n--------------------------------\n\n";
                return false;
            }
            // if version s 2.x.x
        }
    }
}

bool update(std::string latest_ver)
{
    std::string file = "MPCS-" + latest_ver + "-win64.exe";
    std::ifstream myfile;
    myfile.open(file);
    if (!myfile)
    {
        std::ofstream of(file, std::ios::binary);
        std::string installer = "https://github.com/astrohr/MPCS/releases/download/v" + latest_ver + "/" + file;
        std::cout << "Downloading installer from " << installer << std::endl;
        cpr::Response r = cpr::Download(of, cpr::Url{installer});
        of.close();
        std::cout << "http status code = " << r.status_code << std::endl
                  << std::endl;
        if (r.status_code == 200)
        {
            run(latest_ver);
            return true;
        }
        else
        {
            std::cout << "Update failed! Please try again later.";
            return false;
        }
    }
    else
    {
        std::cout << "Installer already exists. Running installer..." << std::endl;
        run(latest_ver);
        return true;
    }
}

void run(std::string latest_ver)
{
#if _WIN32
    std::string command = std::format("start MPCS-{}-win64.exe", latest_ver);
    int status = std::system(command.c_str());
#elif __linux__ // we'll cross that bridge when we get to it

#elif __APPLE__

#endif
    std::cout << "Continue the process in the installer. Closing MPCS...";
}