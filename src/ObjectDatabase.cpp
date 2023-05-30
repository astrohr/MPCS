// ObjectDatabase.cpp --------------------------------------
// this file defines all things declared in ObjectDatabase.hpp

#include "ObjectDatabase.hpp"

//----------------------------------------------------------



std::string ObjectDatabase::b10_to_b26(int c){
    std::string str = "";
    while(c || str == ""){
        if (c%26) {
            str += 'a' + c%26-1;
            c /= 26;
            continue;
        }
        str += 'z';
        c = c/26 - 1;
    } std::reverse(str.begin(), str.end());
    return str;
}

//this function calculates the percentage of datapoints selected by picture areas
void ObjectDatabase::calculateSelected(){
    int covered_amm = 0;
    //making a test for every datapoint
    for(int i = 0; i < obj_data.size(); i++){
        float ephRa, ephDec;    //datapoint coords init
        std::tie(ephRa, ephDec) = obj_data[i].offsets();
        //here we check if the datapoint is within any picture areas
        for(int j = 0; j < pictures.size(); j++){
            float picRa, picDec;    //pic coords init
            std::tie(picRa, picDec) = pictures[j].offsets();
            //the check
            if (abs(picRa-ephRa) < telescope_FOV/2.f && abs(picDec-ephDec) < telescope_FOV/2.f){
                covered_amm++;
                break;
            }
        }
    }
    selectedPercentage = (float)covered_amm / obj_data.size() * 100.f;
}

const std::tuple<float, float> ObjectDatabase::mean_center() const { return {m_mean_centerRa, m_mean_centerDec}; }
const std::tuple<float, float> ObjectDatabase::mean_edges() const { return {m_mean_edgeRa, m_mean_edgeDec}; }
const float ObjectDatabase::selectedPercent() const { return selectedPercentage; }
const std::string ObjectDatabase::name() const { return m_name; }

void ObjectDatabase::set_FOV(int FOV) { telescope_FOV = FOV; }

void ObjectDatabase::set_exposure(int exp){ m_picExposure = exp; }
void ObjectDatabase::set_ammount(int amm){ m_picAmmount = amm; }

const int ObjectDatabase::ephemeris_in_picture(float ra, float dec){
    //locations are in offset arcseconds
    int num = 0;
    for(int i = 0; i < obj_data.size(); i++){
        float ephRa, ephDec;
        std::tie(ephRa, ephDec) = obj_data[i].offsets();
        if (abs(ephRa-ra) < telescope_FOV/2.f && abs(ephDec-dec) < telescope_FOV/2.f) num++;
    }
    return num;
}

void ObjectDatabase::export_observation_targets(bool copy_cpb){
    std::string targets = "";
    for(int i = 0; i < pictures.size(); i++){
        pictures[i].approx_coords(m_centerRa, m_centerDec);
        float ra, dec;
        std::tie(ra, dec) = pictures[i].coords();
        float raOff, decOff;
        std::tie(raOff, decOff) = pictures[i].offsets();
        
        int ephIndex = closest_ephemeris_index(raOff, decOff);
        if (obj_data[ephIndex].time() == "k") obj_data[ephIndex].follow_link();

        //name
        targets += fmt::format("* {}", m_name);
        targets += (pictures.size()-1) ? fmt::format("_{}", b10_to_b26(i+1)) : ""; // adds the letter that shows the picture index (if needed)
        
        //magnitude and picture info
        targets += fmt::format("    {}    {:02} x {:02} sec\r\n", m_magnitude, m_picAmmount, m_picExposure);

        //time
        targets += obj_data[ephIndex].time();

        //right ascnension
        float ra_whole, ra_min = std::modf(ra, &ra_whole)*60.f, ra_sec = std::modf(ra_min, &ra_min)*60.f;
        targets += fmt::format("{:02.0f} {:02.0f} {:04.1f} ", ra_whole, ra_min, ra_sec);

        //declination
        float dec_whole, dec_min = abs(std::modf(dec, &dec_whole)*60.f), dec_sec = std::modf(dec_min, &dec_min)*60.f;
        targets += fmt::format("{:+03.0f} {:02.0f} {:02.0f} ", dec_whole, dec_min, dec_sec);

        //other data
        targets += obj_data[ephIndex].context()+"\r\n\r\n";
    }
    if (targets.size()){
        fmt::print("\nObservation targets for {}:\n\n{}\n", m_name, targets);
        if (copy_cpb){
            sf::Clipboard::setString(targets);
            fmt::print("Copied to clipboard\n");
        }
    }
}

void ObjectDatabase::insert_data(std::string* str){
    Ephemeris e(*str);
    obj_data.emplace_back(e);
}

void ObjectDatabase::insert_picture(float ra, float dec){
    int num = ephemeris_in_picture(ra, dec);
    Picture p(ra, dec, num);
    pictures.emplace_back(p);
    pictures[pictures.size()-1].set_sign(b10_to_b26(pictures.size()));
    calculateSelected();
}

void ObjectDatabase::remove_picture(int index){
    if (!pictures.size()) return;
    pictures.erase(pictures.begin() + index);
    for(int i = 0; i < pictures.size(); i++)
        pictures[i].set_sign(b10_to_b26(i+1));
    calculateSelected();
}

void ObjectDatabase::undo_picture(){
    if (pictures.size())
        pictures.pop_back();
    calculateSelected();
}

void ObjectDatabase::clear_pictures(){
    pictures.clear();
    selectedPercentage = 0.f;
}

const int ObjectDatabase::closest_ephemeris_index(float ra, float dec){
    float x, y, d = FLT_MAX;
    int ind;
    for(int i = 0; i < obj_data.size(); i++){
        std::tie(x, y) = obj_data[i].offsets();
        float newd = sqrt(pow(ra-x, 2) + pow(dec-y, 2));
        if (newd < d){
            d = newd;
            ind = i;
        }
    }
    return ind;
}

const int ObjectDatabase::closest_picture_index(float ra, float dec){
    float x, y, d = FLT_MAX;
    int ind;
    for(int i = 0; i < pictures.size(); i++){
        std::tie(x, y) = pictures[i].offsets();
        float newd = sqrt(pow(ra-x, 2) + pow(dec-y, 2));
        if (newd < d){
            d = newd;
            ind = i;
        }
    }
    return ind;
}

void ObjectDatabase::reset(){
    obj_data.clear();
    m_name.clear();
    pictures.clear();
    m_mean_centerRa = 0.f; m_mean_centerDec = 0.f;
    m_mean_edgeRa = 0.f; m_mean_edgeDec = 0.f;
}

int ObjectDatabase::fill_database(std::string lynk){
    //Get the object name from the link and set it
    bool writing = false;
    m_name.clear();
    for(int i = 0; i < lynk.size(); i++){
        if (lynk[i] == '?'){
            writing = true;
            i += 5;
        }
        else if (lynk[i] == '&') break;

        if (writing) m_name += lynk[i];
    }

    //downloads the data off the internet
    std::vector<std::string> downloaded;
    int returnvalue = get_html(lynk, &downloaded, 375000.0);
    if (returnvalue != 0) return returnvalue;

    //saves the data
    for(int i = 0; i < downloaded.size(); i++){
        if (downloaded[i].size() < 80) continue;
        insert_data(&(downloaded[i]));
    }

    //finding the extremities and using them to set the mean center/egdes
    int minRa = INT_MAX, maxRa = INT_MIN;
    int minDec = INT_MAX, maxDec = INT_MIN;
    for(int i = 0; i < obj_data.size(); i++){
        int ra, dec;
        std::tie(ra, dec) = obj_data[i].offsets();
        if (ra < minRa) minRa = ra;
        if (ra > maxRa) maxRa = ra;
        if (dec < minDec) minDec = dec;
        if (dec > maxDec) maxDec = dec;
    }
    m_mean_centerRa = (float) (minRa+maxRa)/2;
    m_mean_centerDec = (float) (minDec+maxDec)/2;
    m_mean_edgeRa = std::max(abs(m_mean_centerRa-minRa), abs(m_mean_centerRa-maxRa));
    m_mean_edgeDec = std::max(abs(m_mean_centerDec-minDec), abs(m_mean_centerDec-maxDec));

    if (obj_data.empty()){
        fmt::print("Error: no data\n");
        return 4;
    }
    //here we use the fact that the first element has offsets of 0, 0
    returnvalue = obj_data[0].follow_link();
    if (returnvalue != 0) return 1;
    std::tie(m_centerRa, m_centerDec) = obj_data[0].coords();
    m_magnitude = obj_data[0].mag();
    return 0;
}

ObjectDatabase::ObjectDatabase(){
    m_mean_centerRa = 0.f; m_mean_centerDec = 0.f;
    m_mean_edgeRa = 0.f; m_mean_edgeDec = 0.f;
    selectedPercentage = 0.f;
}
