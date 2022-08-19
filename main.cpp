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

int telescope_FOV;   //telescope_FOV in arcseconds

//------------------------CURL STUFF------------------------

static size_t progress_callback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow){
    if (dltotal <= 0.0) {
        std::cout << dlnow << " bytes downloaded\r" << std::flush;
    }
    else{
        std::cout << '|';
        int bar_size = 50; //this + 7 is the total size
        int fragments = round(dlnow/dltotal*bar_size);
        for(int i = 0; i < bar_size; i++){
            if (i<fragments) std::cout << '-';
            else std::cout << ' ';
        }
        std::cout << "| " << (int)(dlnow/dltotal*100) << "%\r" << std::flush;
    }
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

void get_html(std::string link, std::vector<std::string>* userdata){
    CURL *curl = curl_easy_init();
    (*userdata).emplace_back("");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, read_curl_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, userdata);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
    // curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &data);
    curl_easy_setopt(curl, CURLOPT_URL, link.c_str());

    std::cout << "Downloading data...\n";
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) std::cout<< curl_easy_strerror(res) << std::endl;
    else std::cout << "\nDownload success\n" << std::endl;
    
    curl_easy_cleanup(curl);
    return;
}


//--------------------------CLASSES-------------------------

class Ephemeris{
private:
    float m_ra, m_dec;
    int m_offsetRa, m_offsetDec;
    int m_category;
    int m_ephemerisNumber;
    std::string m_link;
public:
    //context is a temporary variable that gives an example of what a ephemeris looks like
    std::string m_context;
    const std::tuple<float, float> coords() const { return {m_ra, m_dec}; }
    const std::tuple<float, float> offsets() const { return {m_offsetRa, m_offsetDec}; }
    const int category() const { return m_category; }

    void approx_coords(float centerRa, float centerDec){
        m_ra = centerRa + (float)m_offsetRa/3600.f/15.f;
        m_dec = centerDec + (float)m_offsetDec/3600.f;
    }

    void follow_link(){
        std::vector<std::string> downloaded;
        get_html(m_link, &downloaded);

        //for now it just looks at the first ephemeris, but that will be changed
        for (int i = 0; i < downloaded.size(); i++){
            if (downloaded[i][0] == '2'){
                //here we use the fact that on the website all data is always equaly spaced
                std::string ra = downloaded[i].substr(18, 10);
                std::string dec = downloaded[i].substr(29, 9);
                
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
        m_category = 0;
        while(raw[i] != '\n'){
            m_category++;
            i++;
        }
    }
};

class Picture{
private:
    float m_ra, m_dec; // not implemented yet
    float m_offsetRa, m_offsetDec;
public:
    const std::tuple<float, float> coords() const { return {m_ra, m_dec}; }
    const std::tuple<float, float> offsets() const { return {m_offsetRa, m_offsetDec}; }

    void approx_coords(float centerRa, float centerDec){
        m_ra = centerRa + (float)m_offsetRa/3600.f/15.f;
        m_dec = centerDec + (float)m_offsetDec/3600.f;
    }

    Picture(float ra, float dec) : m_offsetRa(ra), m_offsetDec(dec) {}
};

class ObjectDatabase{
private:
    // mean center is the offset coordinates of the middle point
    float m_mean_centerRa, m_mean_centerDec;
    // mean edge is the offset distance of points most distant from mean center
    float m_mean_edgeRa, m_mean_edgeDec;
    // center is the absolute coordinates of the 0, 0 ephemerid
    float m_centerRa, m_centerDec;
public:
    std::vector<Ephemeris> obj_data;
    std::vector<Picture> pictures;
    std::string obj_name;

    const std::tuple<float, float> mean_center() const { return {m_mean_centerRa, m_mean_centerDec}; }
    const std::tuple<float, float> mean_edges() const { return {m_mean_edgeRa, m_mean_edgeDec}; }

    void export_observation_targets(){
        //this context thing is just a temporary workaround
        std::string context = obj_data[0].m_context;
        std::string targets = "";
        for(int i = 0; i < pictures.size(); i++){
            pictures[i].approx_coords(m_centerRa, m_centerDec);
            bool wrote = false;
            for (int j = 0; j < context.size(); j++){
                if (j < 18){
                    if (context[j] == ' ') targets += ' ';
                    else targets += context[j];
                }
                else if(j > 37){
                    if (context[j] == ' ') targets += ' ';
                    else targets += 'x';
                }
                else if (!wrote){
                    float ra, dec;
                    std::tie(ra, dec) = pictures[i].coords();
                    std::cout << ra << " " << dec << std::endl;
                    int ra_whole = ra, ra_min = ((float)ra-ra_whole)*60.f, ra_sec = (((float)ra-ra_whole)*60.f - ra_min)*600.f;
                    targets+=('0'+ra_whole/10); targets += ('0'+ra_whole%10); targets += ' '; targets+=('0'+ra_min/10); targets+=('0'+ra_min%10);
                    targets+=' '; targets+=('0'+ra_sec/100); targets+=('0'+(ra_sec%100)/10); targets+='.'; targets+=('0'+ra_sec%10); targets+=' ';

                    int dec_whole = dec, dec_min = ((float)dec-dec_whole)*60.f, dec_sec = (((float)dec-dec_whole)*60.f - dec_min)*60.f;
                    char sajn = (dec < 0) ? '-' : '+';
                    targets+=sajn; targets+=('0'+dec_whole/10); targets+=('0'+dec_whole%10); targets+=' '; targets+=('0'+dec_min/10);
                    targets+=('0'+dec_min%10); targets+=' '; targets+=('0'+dec_sec/10); targets+=('0'+dec_sec%10);
                    wrote = true;
                }
            }
            targets += '\n';
        }
        if (targets.size()){
            sf::Clipboard::setString(targets);
            std::cout << "Observation targets for " + obj_name + ":\n\n" << targets << "\n\nCopied to clipboard" << std::endl;
        }
    }

    void insert_data(std::string* str){
        Ephemeris e(*str);
        obj_data.emplace_back(e);
    }

    void insert_picture(float ra, float dec){
        Picture p(ra, dec);
        pictures.emplace_back(p);
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

    void fill_database(std::string lynk){
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
        get_html(lynk, &downloaded);

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
        obj_data[0].follow_link();
        std::tie(m_centerRa, m_centerDec) = obj_data[0].coords();

        return;
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
        m_zoomFactor = std::min((float)W/edgeRa, (float)H/edgeDec);
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
    using namespace sf;

    //init window

    ContextSettings settings;
    settings.antialiasingLevel = 8;
    VideoMode desktop = VideoMode::getDesktopMode();
    RenderWindow window(VideoMode(cam.window_w(), cam.window_h(), desktop.bitsPerPixel), database.obj_name, Style::Default, settings);
    window.setFramerateLimit(60);

    View view(Vector2f(cam.raOffset(), cam.decOffset()), Vector2f(cam.view_w(), cam.view_h()));
    view.rotate(180);

    Font font; font.loadFromFile("resources/arial.ttf");

    Text infoText;
    infoText.setFont(font);
    infoText.setCharacterSize(20);
    infoText.setRotation(180.f);

    bool fokus = true;          //is window focused?
    bool mistu = false;         //is mouse on the window?
    float mouseRa = 0.f, mouseDec = 0.f;    //where is the mouse?
    bool brisanje = false;      //is a square being deleted?

    while(window.isOpen()){
        //Event processing 
        Event event;
        while(window.pollEvent(event)){
            if(event.type == Event::Closed) window.close();
            if(event.type == Event::KeyPressed) {
                if (event.key.code == Keyboard::Q) window.close();
                if (event.key.code == Keyboard::C) database.pictures.clear();
                if (event.key.code == Keyboard::U && !database.pictures.empty()) database.pictures.pop_back();
                if (event.key.code == Keyboard::R){
                    cam.reset_position();
                    view.setSize(Vector2f(cam.view_w(), cam.view_h()));
                    view.setCenter(cam.raOffset(), cam.decOffset());
                }
            }
            if(event.type == Event::Resized){
                cam.change_dimensions(event.size.width, event.size.height);
                cam.reset_position();
                view.setSize(Vector2f(cam.view_w(), cam.view_h()));
            }
            if (event.type == Event::MouseButtonPressed){
                if (event.mouseButton.button == Mouse::Left){
                    double xd, yd;
                    std::tie(xd, yd) = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
                    database.insert_picture(xd, yd);
                }
                if (event.mouseButton.button == Mouse::Right && !database.pictures.empty()) brisanje = true;
            }
            if (event.type == Event::MouseButtonReleased){
                if (event.mouseButton.button == Mouse::Right && brisanje){
                    double xd, yd;
                    std::tie(xd, yd) = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
                    database.pictures.erase(database.pictures.begin()+database.closest_picture_index(xd, yd));
                    brisanje = false;
                }
            }
            if (event.type == Event::MouseWheelScrolled){
                if (event.mouseWheelScroll.wheel == Mouse::VerticalWheel){
                    cam.change_zoom(event.mouseWheelScroll.delta, event.mouseWheelScroll.x, event.mouseWheelScroll.y);
                    view.setCenter(cam.raOffset(), cam.decOffset());
                    view.setSize(Vector2f(cam.view_w(), cam.view_h()));
                }
            }
            if (event.type == Event::MouseMoved) std::tie(mouseRa, mouseDec) = cam.px_to_off(event.mouseMove.x, event.mouseMove.y);
            if (event.type == Event::LostFocus) fokus = false;
            if (event.type == Event::GainedFocus) fokus = true;
            if (event.type == Event::MouseEntered) mistu = true;
            if (event.type == Event::MouseLeft) mistu = false;
        }
        window.clear();
        window.setView(view);

        //draw dots
        CircleShape tocka(1.5f/cam.zoom());
        for(int i = 0; i < database.obj_data.size(); i++){
            short int cat = database.obj_data[i].category();
            if (cat == 0) tocka.setFillColor(Color(0, 255, 0));
            else if (cat == 2) tocka.setFillColor(Color(255, 255, 0));
            else if (cat == 3) tocka.setFillColor(Color(255, 0, 0));
            else if (cat == 4) tocka.setFillColor(Color(255, 255, 255));
            else if (cat == 11) tocka.setFillColor(Color(0, 0, 255));
            else tocka.setFillColor(Color(255, 0, 255));

            double x, y;
            std::tie(x, y) = database.obj_data[i].offsets();
            tocka.setPosition(x, y);
            window.draw(tocka);
        }

        //setup the square projection settings
        RectangleShape kvadrat(Vector2f(telescope_FOV, telescope_FOV));
        kvadrat.setFillColor(Color::Transparent);
        kvadrat.setOutlineThickness(2.f/cam.zoom());

        //draw a blue square on cursor location
        if (mistu && fokus){
            kvadrat.setOutlineColor(Color(0, 255, 255));
            kvadrat.setPosition(mouseRa-telescope_FOV/2, mouseDec-telescope_FOV/2);
            window.draw(kvadrat);
        }

        //draw other squares
        kvadrat.setOutlineColor(Color(255, 255, 0));
        for(int i = 0; i < database.pictures.size(); i++){
            float xd, yd;
            std::tie(xd, yd) = database.pictures[i].offsets();
            kvadrat.setPosition(xd-telescope_FOV/2, yd-telescope_FOV/2);
            window.draw(kvadrat);
        }

        //show which square will be deleted if the button is released
        if (brisanje){
            int ind = database.closest_picture_index(mouseRa, mouseDec);
            float xd, yd;
            std::tie(xd, yd) = database.pictures[ind].offsets();
            sf::Vertex line[] = {
                Vertex(Vector2f(mouseRa, mouseDec)), 
                Vertex(Vector2f(xd, yd))
            };
            window.draw(line, 2, Lines);
        }

        //show info text
        std::string xs = std::to_string(mouseRa);
        std::string ys = std::to_string(mouseDec);
        infoText.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
        infoText.setPosition(cam.raOffset()+cam.view_w()/2, cam.decOffset()+cam.view_h()/2);
        infoText.setString("Offsets:\nRa= " + xs + "\nDec= " + ys);
        window.draw(infoText);


        window.display();
    }
}


//-----------------------MAIN-----------------------

void defaultVariables()
{
    using namespace std;

    ifstream ReadFile("data/variables.txt");
    string linija;
    while(getline(ReadFile, linija))
    {
        if (linija.size()){
        if (linija[0] == 'F')
        {
            string a = linija.substr(5, linija.length()-5);
            stringstream ss(a);
            ss >> telescope_FOV;
        }
        else
        {
            string a = linija.substr(3, linija.length()-3);
            stringstream ss(a);
            int W, H;
            if (linija[0] == 'H') ss >> H;
            else if (linija[0] == 'W') ss >> W;
            cam.change_dimensions(W, H);
        }
        }
    }
}

int main(void){
    while(true){
        defaultVariables();

        std::cout << "Insert the website URL: ";
        std::string url;
        std::getline(std::cin, url);
        std::cout << std::endl;
        database.fill_database(url);
        std::cout << "Object: " << database.obj_name << std::endl;

        cam.reset_position();

        WindowSetup();
        database.export_observation_targets();
        database.reset();

        std::cout << "\n\n" << std::flush;
    }
    return 0;
}

//TODO:
// Try to make an algorythm for automatically selecting squares in an optimal manner
// Add exposure time variable
// Make ephemeris save their positons change trough time
// Implement time passing display (and linear approximations for movements between hours)
// Add object selection menu
// Remove the console aspect of the app
// Add the windoww to see the changes
// Add panning
// Show the coordinate system
// Add error handling
// Optimize event conditional logic
// remove using namespace calls
// do some inheritance for camera, picture and ephemeris
// break up code
// add help gui
// make downloading progress more interesting