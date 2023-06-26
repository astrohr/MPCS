//----------------------------------------------------------

#include "ObjectDatabase.hpp"

//----------------------------------------------------------



std::string ObjectDatabase::b10_to_b26(int c)
{
    std::string str;
    while(c || str.empty()){
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

const float ObjectDatabase::calculateSelected() const
{
    int covered_amm = 0;
    // making a test for every datapoint
    for(auto dpoint : m_ephemerides){
        auto [ephRa, ephDec] = dpoint.getOffsets();

        // here we check if the datapoint is within any picture areas
        for(auto pic : m_pictures){
            auto [picRa, picDec] = pic.getOffsets();

            // the check
            if (abs(picRa-ephRa) < m_telescope_FOV/2.f && abs(picDec-ephDec) < m_telescope_FOV/2.f){
                covered_amm++;
                break;
            }
        }
    }

    return covered_amm * 100.f / m_ephemerides.size();
}

const int ObjectDatabase::ephemeris_in_picture(float& ra, float& dec)
{
    // locations are in offset arcseconds
    int num = 0;
    for(int i = 0; i < m_ephemerides.size(); i++){
        auto [ephRa, ephDec] = m_ephemerides[i].getOffsets();
        if (abs(ephRa-ra) < m_telescope_FOV/2.f && abs(ephDec-dec) < m_telescope_FOV/2.f) num++;
    }
    return num;
}

void ObjectDatabase::export_observation_targets(bool copy_cpb)
{
    std::string targets;
    for(int i = 0; i < m_pictures.size(); i++)
    {
        m_pictures[i].approxCoords(m_centerRa, m_centerDec);
        auto [ra, dec] = m_pictures[i].getCoords();
        auto [raOff, decOff] = m_pictures[i].getOffsets();
        
        int ephIndex = closest_ephemeris_index(raOff, decOff);
        if (!m_ephemerides[ephIndex].linkVisited()){
            int r = m_ephemerides[ephIndex].follow_link();
            // make sure we dont crash a program with a link that does not work
            if (r != 0) continue;
        }

        // name
        targets += fmt::format("* {}", m_name);
        targets += (m_pictures.size()-1) ? fmt::format("_{}", b10_to_b26(i+1)) : ""; // adds the letter that shows the picture index (if needed)
        
        // magnitude and picture info
        targets += fmt::format("   {:5.1f}    {:02} x {:02} sec\r\n", m_ephemerides[ephIndex].getMag(), m_picAmount, m_picExposure);

        // time
        auto [year, month, day, hour, minute] = Ephemeris::JD_to_date(m_ephemerides[ephIndex].getJDTime());
        targets += fmt::format("{:04} {:02} {:02} {:02}{:02}   ", year, month, day, hour, minute);

        // right ascension
        float ra_whole, ra_min = std::modf(ra, &ra_whole)*60.f, ra_sec = std::modf(ra_min, &ra_min)*60.f;
        targets += fmt::format("{:02.0f} {:02.0f} {:04.1f} ", ra_whole, ra_min, ra_sec);

        // declination
        float dec_whole, dec_min = abs(std::modf(dec, &dec_whole)*60.f), dec_sec = std::modf(dec_min, &dec_min)*60.f;
        targets += fmt::format("{:+03.0f} {:02.0f} {:02.0f} ", dec_whole, dec_min, dec_sec);

        // other data
        targets += m_ephemerides[ephIndex].getContext()+"\r\n\r\n"; // \r\n because thats how newline is saved in the clipboard
    }
    
    if (targets.size())
    {
        fmt::print("\nObservation targets for {}:\n\n{}", m_name, targets);
        if (copy_cpb){
            sf::Clipboard::setString(targets);
            fmt::print("Copied to clipboard\n");
        }
        else{
            fmt::print("! Didnt copy to clipboard !\n");
        }
    }
}

void ObjectDatabase::insert_data(std::string& str)
{
    try{
        Ephemeris eph(str);
        m_ephemerides.emplace_back(eph);
    }
    catch (utils::ConstructorFail& e){
        fmt::print("Warning: Ephemeris Construction failed: \n{}\n", e.what());
    }
}

void ObjectDatabase::insert_picture(float& ra, float& dec)
{
    int num = ephemeris_in_picture(ra, dec);
    Picture p(ra, dec, num);
    m_pictures.emplace_back(p);
    m_pictures[m_pictures.size()-1].setSign(b10_to_b26(m_pictures.size()));
}

void ObjectDatabase::remove_picture(int index)
{   
    if (!m_pictures.size()) return;
    m_pictures.erase(m_pictures.begin() + index);
    for(int i = 0; i < m_pictures.size(); i++)
        m_pictures[i].setSign(b10_to_b26(i+1));
}

void ObjectDatabase::undo_picture()
{
    if (!m_pictures.empty()) m_pictures.pop_back();
}

void ObjectDatabase::clear_pictures()
{
    m_pictures.clear();
}

const int ObjectDatabase::closest_ephemeris_index(float& ra, float& dec)
{
    float d = FLT_MAX;
    int ind;
    for(int i = 0; i < m_ephemerides.size(); i++){
        auto [x, y] = m_ephemerides[i].getOffsets();
        float newd = sqrt(pow(ra-x, 2) + pow(dec-y, 2));
        if (newd < d){
            d = newd;
            ind = i;
        }
    }
    return ind;
}

const int ObjectDatabase::closest_picture_index(float& ra, float& dec)
{
    float d = FLT_MAX;
    int ind;
    for(int i = 0; i < m_pictures.size(); i++){
        auto [x, y] = m_pictures[i].getOffsets();
        float newd = sqrt(pow(ra-x, 2) + pow(dec-y, 2));
        if (newd < d){
            d = newd;
            ind = i;
        }
    }
    return ind;
}

void ObjectDatabase::reset()
{
    m_ephemerides.clear();
    m_name.clear();
    m_pictures.clear();
}

const std::tuple<float, float> ObjectDatabase::getCenter(int* minRa_ptr, int* maxRa_ptr, int* minDec_ptr, int* maxDec_ptr) const
{
    // finding the extremities and using them to set the mean center/egdes
    int minRa = INT_MAX, maxRa = INT_MIN;
    int minDec = INT_MAX, maxDec = INT_MIN;
    for(auto dpoint : m_ephemerides){
        auto [ra, dec] = dpoint.getOffsets();
        if (ra < minRa) minRa = ra;
        if (ra > maxRa) maxRa = ra;
        if (dec < minDec) minDec = dec;
        if (dec > maxDec) maxDec = dec;
    }

    // save edges if so requested
    if (minRa_ptr != nullptr) *minRa_ptr = minRa;
    if (maxRa_ptr != nullptr) *maxRa_ptr = maxRa;
    if (minDec_ptr != nullptr) *minDec_ptr = minDec;
    if (maxDec_ptr != nullptr) *maxDec_ptr = maxDec;

    // center is set in the middle of the extremes
    return {
        (minRa+maxRa)/2.f,
        (minDec+maxDec)/2.f
    };
}

const std::tuple<float, float> ObjectDatabase::getEdges() const
{
    int minRa, maxRa, minDec, maxDec;
    auto [centerRa, centerDec] = this->getCenter(&minRa, &maxRa, &minDec, &maxDec);
    return{  
        std::max(abs(centerRa-minRa), abs(centerRa-maxRa)),
        std::max(abs(centerDec-minDec), abs(centerDec-maxDec))
    };
}

int ObjectDatabase::fill_database(std::string& link)
{
    // Get the object name from the link and set it
    std::smatch match;
    std::regex_search(link, match, std::regex("\\?Obj=(.*?)\\&"), std::regex_constants::match_not_null);
    if (match.size() == 2){
        m_name = match.str(1);
    }
    else {
        fmt::print(
            "Error: provided link has no object name\n"
            "Regex Search found:\n"
        );
        for (auto m : match)
            fmt::print("{} \n", m.str());
        return 2;
    }

    // downloads the data off the internet
    std::vector<std::string> downloaded;
    try{
        utils::get_html(link, downloaded);
    }
    catch (std::exception& e) {
        fmt::print("Error: {}", e.what());
        return 1;
    }

    // saves the data
    for(int i = 0; i < downloaded.size(); i++){
        if (downloaded[i].size() < 80) continue;
        insert_data(downloaded[i]);
    }

    if (m_ephemerides.empty()){
        fmt::print("Error: provided link has no data\n");
        return 1;
    }

    // here we use the fact that the first element has offsets of 0, 0
    int r =  m_ephemerides[0].follow_link();
    // if an error occured return 1
    if (r != 0) return 1;

    std::tie(m_centerRa, m_centerDec) = m_ephemerides[0].getCoords();
    return 0;
}
