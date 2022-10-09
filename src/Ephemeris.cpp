#include "Ephemeris.hpp"

const std::tuple<float, float> Ephemeris::coords() const { return {m_ra, m_dec}; }
const std::tuple<float, float> Ephemeris::offsets() const { return {m_offsetRa, m_offsetDec}; }
const std::string Ephemeris::mag() const { return m_magnitude; }
const std::string Ephemeris::time() const { return m_time; }
const std::string Ephemeris::context() const {
    return m_elong + "  " + m_magnitude + "  " + m_velocity + "  " + m_angle + m_otherData; 
}
const sf::Color Ephemeris::color() const { return m_color; }

void Ephemeris::approx_coords(float centerRa, float centerDec){
    m_ra = centerRa + (float)m_offsetRa/3600.f/15.f;
    m_dec = centerDec + (float)m_offsetDec/3600.f;
    while(m_ra >= 24.f) m_ra -= 24.f;
    while(m_ra < 0.f) m_ra += 24.f;
    while(m_dec > 90.f || m_dec < -90.f){
        if (m_dec > 90.f) m_dec = 180.f-m_dec;
        if (m_dec < -90.f) m_dec = -180.f-m_dec;
    }
}

int Ephemeris::follow_link(){
    std::vector<std::string> downloaded;
    int returnvalue = 1;
    for(int i = 0; i < 3 && returnvalue; i++){
        if (i) std::cout << "Metadata download failed.. trying again" << std::endl;
        returnvalue = get_html(m_link, &downloaded, 4500.0);
    }
    if (returnvalue != 0){
        std::cout << "Metadata download failed, insert the object data yourself" << std::endl;
        std::cout << "!Make sure there is no leading spaces!" << std::endl;
        std::string s = "";
        while(s != "") std::getline(std::cin, s);
        downloaded.emplace_back(s);
    }

    //for now it just looks at the first ephemeris, but that will be changed
    for (int i = 0; i < downloaded.size(); i++){
        if (downloaded[i][0] == '2'){
            //here we use the fact that on the website all data is always equaly spaced
            m_time = downloaded[i].substr(0, 18);
            std::string ra = downloaded[i].substr(18, 10);
            std::string dec = downloaded[i].substr(29, 9);
            m_elong = downloaded[i].substr(39, 5);
            m_magnitude = downloaded[i].substr(46, 4);
            m_velocity = downloaded[i].substr(52, 6);
            m_angle = downloaded[i].substr(60, 5);
            m_otherData = downloaded[i].substr(65, downloaded[i].size()-65);

            std::stringstream streamRa(ra), streamDec(dec);
            
            float ra_whole, ra_min, ra_sec;
            float dec_whole, dec_min, dec_sec;
            
            streamRa >> ra_whole >> ra_min >> ra_sec;
            streamDec >> dec_whole >> dec_min >> dec_sec;

            m_ra = ra_whole + ra_min/60.f + ra_sec/3600.f;
            m_dec = (abs(dec_whole) + dec_min/60.f + dec_sec/3600.f) * (abs(dec_whole)/dec_whole);

            m_context = downloaded[i];
            break;
        }
    }
    return 0;
}

//the constructor is passed the string that describes the ephemeris in the source and parses trough it
Ephemeris::Ephemeris(std::string raw){
    int i = 0;

    // getting the offsets
    std::string n1 = "", n2 = "";
    while(raw[i] != '<'){
        if (raw[i] == '-' || raw[i] == '+'){
            if (n1.empty()) n1 += raw[i];
            else n2 += raw[i];
        }
        else if (raw[i] != ' '){
            if (n2.empty()) n1 += raw[i];
            else n2 += raw[i];
        }
        i++;
    }
    std::stringstream stream1(n1), stream2(n2);
    stream1 >> m_offsetRa; stream2 >> m_offsetDec;
    while(raw[i-1] != '"') i++;
    
    //getting the link of the ephemeris
    while(raw[i] != '"'){
        m_link += raw[i];
        i++;
    }
    while(raw[i-2] != '#') i++;

    //getting the number of the ephemeris
    m_ephemerisNumber = 0;
    while(raw[i] != '<'){
        m_ephemerisNumber = m_ephemerisNumber*10 + (raw[i]-'0');
        i++;
    }
    while(raw[i-1] != '>') i++;

    //getting the category
    int cat = 0;
    while(raw[i] != '\n'){
        cat++; 
        i++;
    }
    
    if (cat == 0) m_color = sf::Color(0, 255, 0);
    else if (cat == 2) m_color = sf::Color(255, 255, 0);
    else if (cat == 3) m_color = sf::Color(255, 0, 0);
    else if (cat == 4) m_color = sf::Color(255, 255, 255);
    else if (cat == 11) m_color = sf::Color(0, 0, 255);
    else m_color = sf::Color(255, 0, 255);
    
    m_time = "k"; //this just means the link hasnt been followed yet
}