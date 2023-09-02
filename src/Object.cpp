//----------------------------------------------------------

#include "Object.hpp"

//----------------------------------------------------------


Object::Object(
    std::string& name, int& score, time_t& discoveryTime, CoordinatesSky& coords, float& magnitude,
    time_t& lastUpdate, std::string& note, int& nObs, float& arc, float& H, float& notSeen
)
: name(name), score(score), discoveryTime(discoveryTime), coords(coords), magnitude(magnitude),
    lastUpdate(lastUpdate), note(note), nObs(nObs), arc(arc), H(H), notSeen(notSeen)
{
    coords3d = skyTo3D(coords);
}

void Object::extractOrbits()
{
    std::string orbitsLink = "https://cgi.minorplanetcenter.net/cgi-bin/showobsorbs.cgi?Obj=" + this->name + "&orb=y";
    std::vector<std::string> raw;
    get_html(orbitsLink, raw);

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
            throw mpcsError::BadData(std::format("Bad data found on {}: \n{}", orbitsLink, e.what()));
        }

        // instead of trowing an error here (and in other places), write the data to a log and continue scanning
    	
        // create the orbit
        this->variantOrbits.emplace_back(H, G, epoch, M, perihelion, node, inclination, e, n, a, nOpp, arc, rms, orbitID);
    }

}


void Object::findObjects(std::vector<Object>& objects)
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
        int score, year, month; float day, mag; double raH, dec;
        int nObs; float arc, H, dns;

        try{
            ss1 >> name;

            ss1 >> temp; score = std::stoi(temp);
            ss1 >> temp; year = std::stoi(temp);
            ss1 >> temp; month = std::stoi(temp);
            ss1 >> temp; day = std::stof(temp);
            ss1 >> temp; raH = std::stod(temp);
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
        float hour = (day - (float)((int)day)) * 24.f;
        float minute = (hour - (float)((int)hour)) * 60.f;
        time_t discoveryTime = date_to_unix_milis(year, month, day, hour, minute, 0);

        // make a coordinates variable
        CoordinatesSky coords = {raH*15.0, dec}; // times 15 because its provided in hours, not degrees

        // turn the lastUpdate string into Time

        std::stringstream ss3(lastUpdateStr);

        float time;
        int year2 = 1970, month2; // year is 1900 because year isnt provided and its not needed to specify it in this case
        float day2;
        try{          
            ss3 >> temp; ss3 >> temp; 
            if (temp.back() == '.') temp.pop_back(); // remove the dot from the end of the month
            month2 = monthVals[temp];
            ss3 >> temp;
            day2 = std::stof(temp);
        } catch(std::exception& e){
            throw mpcsError::BadData(std::format("Bad data found on {}: \n{}", objectsLink, e.what()));
        }
        float hour2 = (day2 - (float)((int)day2)) * 24.f;
        float minute2 = (hour2 - (float)((int)hour2)) * 60.f;
        time_t lastUpdate = date_to_unix_milis(year2, month2, day2, hour2, minute2, 0);

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