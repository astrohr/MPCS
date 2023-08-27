//----------------------------------------------------------

#include "Observatory.hpp"

//----------------------------------------------------------

bool Observatory::fillData()
{
    std::string observatoryLinks = "https://minorplanetcenter.net/iau/lists/ObsCodes.html";
    std::vector<std::string> raw;
    get_html(observatoryLinks, raw);

    // a single string looks like this
    // Code  Long.   cos      sin    Name
    // 000   0.0000 0.62411 +0.77873 Greenwich

    // note that separating things with a space is not guaranteed here
    // code is always 3 letters
    // numbers have max 6 decimal digits

    double longitude, cos, sin;
    std::string name;

    for(auto& line : raw){
        if (line.size() < 4) continue;
        std::string code = line.substr(0, 3);
        if (code != this->ID) continue;

        try{
            longitude = std::stod(line.substr(4, 9));
            cos = std::stod(line.substr(13, 8));
            sin = std::stod(line.substr(21, 9));
            name = line.substr(30);
        } catch (std::exception& e){
            throw mpcsError::BadData(std::format("Bad data found on {}: \n{}", observatoryLinks, e.what()));
        } 
    }

    // calculate latitude
    double latitude = 180.0/std::numbers::pi * (std::atan(sin/cos));

    this->name = name;
    this->coords = {longitude, latitude};

    // write data to MPCS.ini
    // note that this part of code RELIES on previous code to check there is an observatory section with its id specified
    std::fstream iniFile(g_mpcsIniPath);
    if(!iniFile) return 1;

    std::stringstream ss; ss << iniFile.rdbuf(); // copy the file
    std::string data = ss.str();
    
    size_t obsData = data.find("[Observatory]");
    size_t nextData = data.find('[', obsData+1); // find where the next section begins
    if (nextData == std::string::npos) nextData = data.size(); // and if its not there just mark the end
    std::string dataWindow = data.substr(obsData, nextData-obsData); // isolate this section
    size_t idData = dataWindow.find("ID");
    size_t nameData = dataWindow.find("NAME");
    bool noName = (nameData == std::string::npos); // check if name parameter exists

    size_t writePosition = (noName) ? obsData+idData+1 : obsData+dataWindow.find("NAME")+1 ;
    while(data[writePosition-1] != '\n') writePosition++; // find the write position
    
    std::string newData = data.substr(0, writePosition);
    if (noName) newData += std::format("NAME = {}\n", name);
    newData += std::format("LATITUDE = {:.6f}\n", latitude);
    newData += std::format("LONGITUDE = {:.6f}\n", longitude);

    std::string line;
    bool spaces = true, comment = false;
    for(int i = writePosition; i < nextData; i++){ // perserve any empty lines or comments
        line += data[i];
        if (data[i] != '\n'){
            if (data[i] == ';') comment = true;
            if (data[i] != ' ') spaces = false;
        }
        else{
            if (spaces || comment) newData += line;
            spaces = true;
            comment = false;
            line = "";
        }
    }

    newData += data.substr(nextData); // and add the rest of the file
    
    std::filesystem::resize_file(g_mpcsIniPath, 0); // clear the file
    iniFile.seekp(0); // go to the beginning
    iniFile << newData; // write

    iniFile.close();
    return 0;
}