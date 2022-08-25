#include <SFML/Graphics.hpp>
#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cfloat>
#include <string>
#include <tuple>
#include <cmath>

#include "cmakevars.h"

int telescope_FOV;   //telescope_FOV in arcseconds

//------------------------CURL STUFF------------------------

std::vector<std::string> allowed_links; //this gets populated in default_variables()

static size_t progress_callback(void* approx_size, double dltotal, double dlnow, double ultotal, double ulnow){
    if (dltotal <= 0.0) dltotal = *((double*)approx_size);
    if (dltotal > 0.0){
        std::cout << '[';
        int bar_size = 45; //this + 15 ish is the total size
        int fragments = round(dlnow/dltotal*bar_size);
        for(int i = 0; i < bar_size; i++){
            if (i<fragments) std::cout << '#';
            else std::cout << '-';
        }
        std::cout << "] " << std::min((int)(dlnow/dltotal*100), 100) << "%  ";
    }
    std::cout << "(" << dlnow << " B)\r" << std::flush;
    return 0;
}

size_t read_curl_data(char* ptr, size_t size, size_t nmemb, std::vector<std::string>* userdata){
    size_t bytes = size*nmemb;

    for(char *i = ptr; i-ptr < bytes; i++){
        (*userdata)[0] += *i;
        if(*i == '\n'){
            (*userdata).emplace_back((*userdata)[0]);
            (*userdata)[0] = "";
        }
    }
    return bytes; // returns the number of bytes passed to the function
}

int get_html(std::string link, std::vector<std::string>* userdata, double size = 0.0){
    CURL *curl = curl_easy_init();
    (*userdata).emplace_back("");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_curl_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, userdata);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, (void*)&size);
    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());

    bool match = false;
    for(int i = 0; i < allowed_links.size() && !match; i++){
        bool ac = true;
        for(int j = 0; j < allowed_links[i].size(); j++){
            if (j >= link.size() || link[j] != allowed_links[i][j]){
                ac = false;
                break;
            }
        }
        if(ac) match = true;
    }
    if (!match){
        std::cout << "Error: link not allowed" << std::endl;
        return 2;
    }

    std::cout << "Downloading data..." << std::endl;
    CURLcode res = curl_easy_perform(curl);
    int returnvalue;
    if (res != CURLE_OK) {
        std::cout<< curl_easy_strerror(res) << std::endl;
        returnvalue = 1;
    }
    else{
        std::cout << "\nDownload success\n" << std::endl;
        returnvalue = 0;
    }
    
    curl_easy_cleanup(curl);
    return returnvalue;
}


//--------------------------CLASSES-------------------------

class Ephemeris{
private:
    float m_ra, m_dec;
    int m_offsetRa, m_offsetDec;
    int m_ephemerisNumber;
    std::string m_link;
    std::string m_magnitude;
    sf::Color m_color;
public:
    //context is a temporary variable that gives an example of what an ephemeris looks like
    std::string m_context;
    const std::tuple<float, float> coords() const { return {m_ra, m_dec}; }
    const std::tuple<float, float> offsets() const { return {m_offsetRa, m_offsetDec}; }
    const std::string mag() const { return m_magnitude; }
    const sf::Color color() const { return m_color; }

    void approx_coords(float centerRa, float centerDec){
        m_ra = centerRa + (float)m_offsetRa/3600.f/15.f;
        m_dec = centerDec + (float)m_offsetDec/3600.f;
        while(m_ra >= 24.f) m_ra -= 24.f;
        while(m_ra < 0.f) m_ra += 24.f;
        while(m_dec > 90.f || m_dec < -90.f){
            if (m_dec > 90.f) m_dec = 180.f-m_dec;
            if (m_dec < -90.f) m_dec = -180.f-m_dec;
        }
    }

    int follow_link(){
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
                std::string ra = downloaded[i].substr(18, 10);
                std::string dec = downloaded[i].substr(29, 9);
                m_magnitude = downloaded[i].substr(46, 4);

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
    Ephemeris(std::string raw){
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
    }
};

class Picture{
private:
    float m_ra, m_dec;
    float m_offsetRa, m_offsetDec;
    int m_containedEphemeris;
    sf::Text m_text;
public:
    const std::tuple<float, float> coords() const { return {m_ra, m_dec}; }
    const std::tuple<float, float> offsets() const { return {m_offsetRa, m_offsetDec}; }
    const sf::Text text() const { return m_text; }
    const std::string sign() const { return (std::string)m_text.getString(); }
    const float percent(int totalEphemeris) const { return (float)m_containedEphemeris/totalEphemeris * 100.f; }
    const std::string percentStr(int totalEphemeris) const {
        float percent = (float)m_containedEphemeris/totalEphemeris * 100.f;
        std::string s = std::to_string(percent);
        s = s.substr(0, s.size()-4) + "%";
        return s;
    }

    void approx_coords(float centerRa, float centerDec){
        m_ra = centerRa + (float)m_offsetRa/3600.f/15.f;
        m_dec = centerDec + (float)m_offsetDec/3600.f;
        while(m_ra >= 24.f) m_ra -= 24.f;
        while(m_ra < 0.f) m_ra += 24.f;
        while(m_dec > 90.f || m_dec < -90.f){
            if (m_dec > 90.f) m_dec = 180.f-m_dec;
            if (m_dec < -90.f) m_dec = -180.f-m_dec;
        }
    }

    void set_sign(std::string sgn){ m_text.setString(sgn); }

    Picture(float ra, float dec, int objNum) : m_offsetRa(ra), m_offsetDec(dec), m_containedEphemeris(objNum) {
        m_text.setCharacterSize(20);
        m_text.setRotation(180.f);
        m_text.setPosition(m_offsetRa, m_offsetDec);
        m_text.setFillColor(sf::Color(255, 255, 0));
    }
};

class ObjectDatabase{
private:
    // mean center is the offset coordinates of the middle point
    float m_mean_centerRa, m_mean_centerDec;
    // mean edge is the offset distance of points most distant from mean center
    float m_mean_edgeRa, m_mean_edgeDec;
    // center is the absolute coordinates of the 0, 0 ephemerid
    float m_centerRa, m_centerDec;
    int m_picExposure, m_picAmmount;
    std::string m_magnitude;

    //function for nicely formatting the numbers to strings (with leading zeroes)
    std::string frmt(int num, int digits=2){
        int dgtnum = 0, num2 = num;
        while(num2){
            dgtnum++;
            num2 /= 10;
        } digits = std::max(dgtnum, digits);
        
        std::string str = "";
        for(int i = digits-1; i >= 0; i--)
            str += '0'+(num/(int)pow(10, i)%10);
        
        return str;
    }

    std::string b10_to_b26(int c){
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

public:
    std::vector<Ephemeris> obj_data;
    std::vector<Picture> pictures;
    std::string obj_name;

    const std::tuple<float, float> mean_center() const { return {m_mean_centerRa, m_mean_centerDec}; }
    const std::tuple<float, float> mean_edges() const { return {m_mean_edgeRa, m_mean_edgeDec}; }

    void set_exposure(int exp){ m_picExposure = exp; }
    void set_ammount(int amm){ m_picAmmount = amm; }

    const int ephemeris_in_picture(float ra, float dec){
        //locations are in offset arcseconds
        int num = 0;
        for(int i = 0; i < obj_data.size(); i++){
            float ephRa, ephDec;
            std::tie(ephRa, ephDec) = obj_data[i].offsets();
            if (abs(ephRa-ra) < telescope_FOV/2.f && abs(ephDec-dec) < telescope_FOV/2.f) num++;
        }
        return num;
    }

    void export_observation_targets(bool copy_cpb){
        //this context thing is just a temporary workaround
        std::string context = obj_data[0].m_context;
        std::string targets = "";
        std::string cpbnl = "\r\n"; //clipboard 
        for(int i = 0; i < pictures.size(); i++){
            pictures[i].approx_coords(m_centerRa, m_centerDec);
            float ra, dec;
            std::tie(ra, dec) = pictures[i].coords();
            if (pictures.size() > 1){
    	        std::string letter = b10_to_b26(i+1);
                targets+="* "+obj_name+"_"+letter+"    "+m_magnitude+"    "+frmt(m_picAmmount)+" x "+frmt(m_picExposure)+" sec"+cpbnl;
            }
            else targets+="* "+obj_name+"    "+m_magnitude+"    "+frmt(m_picAmmount)+" x "+frmt(m_picExposure)+" sec"+cpbnl;

            bool wrote = false;
            for (int j = 0; j < context.size(); j++){
                if (j < 18){
                    if (context[j] == ' ') targets += ' ';
                    else targets += context[j];
                }
                else if(j > 50){
                    if (context[j] == ' ') targets += ' ';
                    else targets += 'x';
                }
                else if (!wrote){
                    int ra_whole = ra, ra_min = ((float)ra-ra_whole)*60.f, ra_sec = (((float)ra-ra_whole)*60.f - ra_min)*600.f;

                    targets += frmt(ra_whole) + " " + frmt(ra_min) + " " + frmt(ra_sec, 3).insert(2, ".") + " ";

                    int dec_whole = abs(dec), dec_min = ((float)abs(dec)-dec_whole)*60.f, dec_sec = (((float)abs(dec)-dec_whole)*60.f - dec_min)*60.f;
                    std::string sajn = (dec < 0) ? "-" : "+";
                    targets += sajn + frmt(dec_whole) + " " + frmt(dec_min) + " " + frmt(dec_sec);
                    targets += " xxxxx  " + m_magnitude;
                    wrote = true;
                }
            }
            targets += cpbnl+cpbnl;
        }
        if (targets.size()){
            std::cout << "\nObservation targets for " + obj_name + ":\n\n" << targets << std::endl;
            if (copy_cpb){
                sf::Clipboard::setString(targets);
                std::cout << "\nCopied to clipboard" << std::endl;
            }
        }
    }

    void insert_data(std::string* str){
        Ephemeris e(*str);
        obj_data.emplace_back(e);
    }

    void insert_picture(float ra, float dec){
        int num = ephemeris_in_picture(ra, dec);
        Picture p(ra, dec, num);
        pictures.emplace_back(p);
        pictures[pictures.size()-1].set_sign(b10_to_b26(pictures.size()));
    }

    void object_remove(int index){
        pictures.erase(pictures.begin() + index);
        for(int i = 0; i < pictures.size(); i++)
            pictures[i].set_sign(b10_to_b26(i+1));
    }

    const int closest_picture_index(float ra, float dec){
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

    void reset(){
        obj_data.clear();
        obj_name.clear();
        pictures.clear();
        m_mean_centerRa = 0.f; m_mean_centerDec = 0.f;
        m_mean_edgeRa = 0.f; m_mean_edgeDec = 0.f;
    }

    int fill_database(std::string lynk){
        //Get the object name from the link and set it
        bool writing = false;
        obj_name.clear();
        for(int i = 0; i < lynk.size(); i++){
            if (lynk[i] == '?'){
                writing = true;
                i += 5;
            }
            else if (lynk[i] == '&') break;

            if (writing) obj_name += lynk[i];
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

        //here we use the fact that the first element has offsets of 0, 0 
        if (obj_data.empty()){
            std::cout << "Error: no data" << std::endl;
            return 4;
        }
        returnvalue = obj_data[0].follow_link();
        if (returnvalue != 0) return 1;
        std::tie(m_centerRa, m_centerDec) = obj_data[0].coords();
        m_magnitude = obj_data[0].mag();

        return 0;
    }

    ObjectDatabase(){
        m_mean_centerRa = 0.f; m_mean_centerDec = 0.f;
        m_mean_edgeRa = 0.f; m_mean_edgeDec = 0.f;
    }

} database;

class Camera{
private:
    //camera location in offset coords
    float m_offsetRa, m_offsetDec;
    //camera location in absolute coords [NOT IMPLEMENTED YET]
    float m_ra_abs, m_dec_abs;
    //zoom factor -> ratio of window size in px and arcseconds that fit in the window
    float m_zoomFactor;
    //window dimensions in px
    int W, H;
public:
    const float raOffset() const { return m_offsetRa; }
    const float decOffset() const { return m_offsetDec; }

    const int window_w() const { return W; }
    const int window_h() const { return H; }

    const float zoom() const { return m_zoomFactor; }
    const float view_w() const { return W/m_zoomFactor; }
    const float view_h() const { return H/m_zoomFactor; }

    void reset_position(){
        std::tie(m_offsetRa, m_offsetDec) = database.mean_center();
        float edgeRa, edgeDec;
        std::tie(edgeRa, edgeDec) = database.mean_edges();
        edgeRa *= 2; edgeDec *= 2;
        edgeRa = std::max(telescope_FOV*1.3f, edgeRa);
        edgeDec = std::max(telescope_FOV*1.3f, edgeDec);
        m_zoomFactor = std::min((float)W/edgeRa, (float)H/edgeDec) * 0.9f;
    }

    void pan_camera(int deltaX, int deltaY){
        m_offsetRa += (float)deltaX * W/m_zoomFactor * 0.005f;
        m_offsetDec += (float)deltaY * H/m_zoomFactor * 0.005f;
    }

    const std::tuple<float, float> px_to_off(int x, int y) {
        float x_to_center = (float)W/2.f - x, y_to_center = (float)H/2.f - y;
        float x_off = x_to_center/m_zoomFactor + m_offsetRa;
        float y_off = y_to_center/m_zoomFactor + m_offsetDec;
        return {x_off, y_off};
    }

    void change_zoom(float delta, float x, float y){
        m_offsetRa += ((W/2.f - x) * 0.05f * delta) / m_zoomFactor;
        m_offsetDec += ((H/2.f - y) * 0.05f * delta) / m_zoomFactor;
        m_zoomFactor += m_zoomFactor * 0.05f * delta;
    }

    void change_dimensions(int x, int y){ W = x; H = y; }

    Camera(int x = 1080, int y = 920) : W(x), H(y) {
        m_offsetRa = 0.f;
        m_offsetDec = 0.f;
    }
} cam;


//-----------------------WINDOW STUFF-----------------------

void WindowSetup(){
    //init window
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(cam.window_w(), cam.window_h(), desktop.bitsPerPixel), database.obj_name, sf::Style::Default, settings);
    window.setFramerateLimit(60);

    sf::View view(sf::Vector2f(cam.raOffset(), cam.decOffset()), sf::Vector2f(cam.view_w(), cam.view_h()));
    view.rotate(180);

    sf::Font font; 
    if (!font.loadFromFile("resources/arial.ttf"))
        std::cout << "Font not found, using default font" << std::endl;

    sf::Text infoText;
    infoText.setFont(font);
    infoText.setCharacterSize(20);
    infoText.setRotation(180.f);

    sf::Text probText;
    probText.setFont(font);
    probText.setCharacterSize(20);
    probText.setRotation(180.f);
    
    sf::RectangleShape kvadrat(sf::Vector2f(telescope_FOV, telescope_FOV));
    kvadrat.setFillColor(sf::Color::Transparent);

    bool fokus = true;                      //is window focused?
    float mouseRa = 0.f, mouseDec = 0.f;    //where is the mouse?
    while(window.isOpen()){
        //Event processing 
        sf::Event event;
        while(window.pollEvent(event)){
            if(event.type == sf::Event::Closed) window.close();
            else if(event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Q) window.close();
                else if (event.key.code == sf::Keyboard::C) database.pictures.clear();
                else if (event.key.code == sf::Keyboard::U && !database.pictures.empty()) database.pictures.pop_back();
                else if (event.key.code == sf::Keyboard::R) cam.reset_position();
                else if (event.key.code == sf::Keyboard::H){
                    std::cout << "\nLeft Click to add an observation target" << std::endl;
                    std::cout << "Right Click to remove an observation target" << std::endl;
                    std::cout << "Q to exit the window and confirm the selection" << std::endl;
                    std::cout << "R to reset the view" << std::endl;
                    std::cout << "U to undo an observation target" << std::endl;
                    std::cout << "C to remove all observation targets" << std::endl;
                    std::cout << "You can zoom in and out with the scroll wheel" << std::endl;
                    std::cout << "Arrow keys for panning the view\n" << std::endl;
                }
            }
            else if(event.type == sf::Event::Resized){
                cam.change_dimensions(event.size.width, event.size.height);
                cam.reset_position();
            }
            else if (event.type == sf::Event::MouseButtonPressed){
                if (event.mouseButton.button == sf::Mouse::Left){
                    double xd, yd;
                    std::tie(xd, yd) = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
                    database.insert_picture(xd, yd);
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased){
                if (event.mouseButton.button == sf::Mouse::Right){
                    double xd, yd;
                    std::tie(xd, yd) = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
                    database.object_remove(database.closest_picture_index(xd, yd));
                }
            }
            else if (event.type == sf::Event::MouseWheelScrolled){
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel){
                    cam.change_zoom(event.mouseWheelScroll.delta, event.mouseWheelScroll.x, event.mouseWheelScroll.y);
                }
            }
            else if (event.type == sf::Event::LostFocus) fokus = false;
            else if (event.type == sf::Event::GainedFocus) fokus = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) cam.pan_camera(-1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) cam.pan_camera(0, -1);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) cam.pan_camera(1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) cam.pan_camera(0, 1);


        window.clear();
        view.setSize(sf::Vector2f(cam.view_w(), cam.view_h()));
        view.setCenter(cam.raOffset(), cam.decOffset());
        window.setView(view);

        sf::Vector2i pos = sf::Mouse::getPosition(window);
        std::tie(mouseRa, mouseDec) = cam.px_to_off(pos.x, pos.y);

        //draw dots
        sf::CircleShape tocka(1.5f/cam.zoom());
        for(int i = 0; i < database.obj_data.size(); i++){
            tocka.setFillColor(database.obj_data[i].color());
            double x, y;
            std::tie(x, y) = database.obj_data[i].offsets();
            tocka.setPosition(x, y);
            window.draw(tocka);
        }

        //setup the square projection settings
        kvadrat.setOutlineThickness(2.f/cam.zoom());

        //draw a blue square on cursor location
        if (fokus){
            kvadrat.setOutlineColor(sf::Color(0, 255, 255));
            kvadrat.setPosition(mouseRa-telescope_FOV/2, mouseDec-telescope_FOV/2);
            window.draw(kvadrat);
        }

        //draw other squares
        for(int i = 0; i < database.pictures.size(); i++){
            float xd, yd;
            std::tie(xd, yd) = database.pictures[i].offsets();
            kvadrat.setPosition(xd-telescope_FOV/2, yd-telescope_FOV/2);
            kvadrat.setOutlineColor(sf::Color(100, 100, 100));
            kvadrat.setOutlineThickness(3.5f/cam.zoom());
            window.draw(kvadrat);
            kvadrat.setOutlineColor(sf::Color(255, 255, 0));
            kvadrat.setOutlineThickness(2.f/cam.zoom());
            window.draw(kvadrat);
            sf::Text kvat = database.pictures[i].text();
            kvat.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
            kvat.setFont(font);
            window.draw(kvat);

        }

        //show which square will be deleted if the button is released
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)){
            int ind = database.closest_picture_index(mouseRa, mouseDec);
            float xd, yd;
            std::tie(xd, yd) = database.pictures[ind].offsets();
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(mouseRa, mouseDec)), 
                sf::Vertex(sf::Vector2f(xd, yd))
            };
            window.draw(line, 2, sf::Lines);
        }

        //show info text
        infoText.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
        infoText.setPosition(cam.raOffset()+cam.view_w()/2.f, cam.decOffset()+cam.view_h()/2.f);
        std::string capturePercent = std::to_string((float)database.ephemeris_in_picture(mouseRa, mouseDec)/database.obj_data.size() * 100.f);
        capturePercent = capturePercent.substr(0, capturePercent.size()-4) + "%";
        std::string mouseRaStr = std::to_string(mouseRa);
        mouseRaStr = mouseRaStr.substr(0, mouseRaStr.size()-4);
        std::string mouseDecStr = std::to_string(mouseDec);
        mouseDecStr = mouseDecStr.substr(0, mouseDecStr.size()-4);
        infoText.setString("Offsets:\nRa: " + mouseRaStr + "\nDec: " + mouseDecStr + "\n\n" + capturePercent);
        window.draw(infoText);

        //show probability text
        probText.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
        probText.setPosition(cam.raOffset()-cam.view_w()*2.5f/7.f, cam.decOffset()+cam.view_h()/2.f);
        std::string buraz = "";
        for(int i = 0; i < database.pictures.size(); i++)
            buraz += database.pictures[i].sign() + " " + database.pictures[i].percentStr(database.obj_data.size()) +  "\n";
        probText.setString(buraz);
        window.draw(probText);

        window.display();
    }
}


//---------------------------MAIN---------------------------

int defaultVariables(){
    int W, H;
    std::string linija;

    std::ifstream ReadFile("data/variables.txt");
    if (!ReadFile.is_open()){
        std::cout << "Error: didnt find \"data/variable.txt\"" << std::endl;
        return 1;
    }
    while(getline(ReadFile, linija)){
        if (!linija.size()) continue;

        if (linija[0] == 'F'){
            std::string a = linija.substr(5, linija.length()-5);
            std::stringstream ss(a);
            ss >> telescope_FOV;
        }
        else{
            std::string a = linija.substr(3, linija.length()-3);
            std::stringstream ss(a);
            if (linija[0] == 'H') ss >> H;
            else if (linija[0] == 'W') ss >> W;
        }
    }
    cam.change_dimensions(W, H);
    ReadFile.close();

    std::ifstream LinksFile("data/allowed_links.txt");
    if (!LinksFile.is_open()){
        std::cout << "Error: didnt find \"data/allowed_links.txt\"" << std::endl;
        return 1;
    }
    while(getline(LinksFile, linija)){
        if(!linija.size()) continue;
        allowed_links.emplace_back(linija);
    }
    LinksFile.close();
    return 0;
}

int main(int argc, char** argv){
    //argc syntax: MPCS <url:str> <exposition:int> <number:int> <copy to clipboard:bool(1/0)>
    //i wont check your inputs, make sure you code them right yourself
    if (argc > 6){
        std::cout << "Error: Too many arguments" << std::endl;
        return 0;
    }
    std::cout << "Running MPCSolver " << VERSION_MAJOR << "." << VERSION_MINOR << "\n" << std::endl;

    if (defaultVariables()) return 1;
    while(true){
        database.reset();

        std::string url;
        if (argc == 1){
            std::cout << "Insert the website URL: " << std::endl;
            //the for is here to make sure that the url inserted isnt an empty line
            for(int i = 0; i < 3 && !url.size(); i++) std::getline(std::cin, url);
            std::cout << std::endl;
        }
        else url = argv[1];
        int greska = database.fill_database(url);
        if (greska){
            std::cout << std::endl << std::endl;
            if (argc == 1) continue;
            else break;
        }
        cam.reset_position();
        
        if (argc == 1) std::cout << "Object: " << database.obj_name << std::endl;

        int amm, exp;
        if (argc < 3){
            std::cout << "Insert the ammount of pictures: " << std::flush;
            std::cin >> amm;
        }
        else amm = atoi(argv[2]);
        database.set_ammount(amm);

        if (argc < 4){
            std::cout << "Insert the exposure length (in seconds): " << std::flush;
            std::cin >> exp;
        }
        else exp = atoi(argv[3]);
        database.set_exposure(exp);

        WindowSetup();

        bool copy_to_clipboard = true;
        if (argc == 5) copy_to_clipboard = atoi(argv[4]);
        database.export_observation_targets(copy_to_clipboard);
        
        if (argc > 1){
            std::cout << "Click any key to exit..." << std::endl;
            std::cin.get();
            break; // if the program was called from the console just kill it after use
        }
        std::cout << std::endl << std::endl;
    }
    return 0;
}

//TODO:
// Try to make an algorythm for automatically selecting squares in an optimal manner
// Make ephemeris save their positons change trough time
// Implement time passing display (and linear approximations for movements between hours)
// Add object selection menu
// Remove the console aspect of the app
// Show the coordinate system
// do some inheritance for camera, picture and ephemeris
// break up code