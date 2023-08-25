//----------------------------------------------------------

#include "Object.hpp"

//----------------------------------------------------------

void Object::extractOrbits()
{
    std::string objectsLink = "https://cgi.minorplanetcenter.net/cgi-bin/showobsorbs.cgi?Obj=" + this->name + "&orb=y";
    std::vector<std::string> raw;
    get_html(objectsLink, raw);

    // a single string looks like this
    // P11Ip0N 18.6  0.15  K238P 356.90249    8.24817  353.26306   12.51383  0.6861987  0.06144459   6.3603394                  4   1    0 days 0.11         NEOCPV0001
    // Object   H     G    Epoch    M         Peri.      Node       Incl.        e           n         a                     NObs NOpp   Arc    r.m.s.       Orbit ID

    // orbit id disclaimer: Nomin = 00001, Vxxxx = xxxx+1
    
    for(auto& line : raw)
    {
        if (line.size() < 160) continue;

        std::stringstream ss(line);
        std::string temp;

        float H, G;
        std::string epoch;
        double M, perihelion, node, inclination, e, n, a;
        int nOpp, arc;
        float rms;
        int orbitID;

        try{
            ss >> temp; // object name
            ss >> temp; H = std::stof(temp);
            ss >> temp; G = std::stof(temp);
            ss >> epoch; 
            ss >> temp; M = std::stod(temp);
            ss >> temp; perihelion = std::stod(temp);
            ss >> temp; node = std::stod(temp);
            ss >> temp; inclination = std::stod(temp);
            ss >> temp; e = std::stod(temp);
            ss >> temp; n = std::stod(temp);
            ss >> temp; a = std::stod(temp);
            ss >> temp; // nobs
            ss >> temp; nOpp = std::stoi(temp);
            ss >> temp; arc = std::stoi(temp);
            ss >> temp; // "days"
            ss >> temp; rms = std::stof(temp);
            ss >> temp; temp = temp.substr(6, 4); orbitID = (temp == "omin") ? 1 : std::stoi(temp) + 1;
        } catch(std::exception& e){
            throw mpcsError::BadData(std::format("Bad data found on {}: \n{}", objectsLink, e.what()));
        }

        // instead of trowing an error here (and in other places), write the data to a log and continue scanning
    	
        // create the orbit
        this->variantOrbits.emplace_back(H, G, epoch, M, perihelion, node, inclination, e, n, a, nOpp, arc, rms, orbitID);
    }

}
