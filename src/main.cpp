// main.cpp ------------------------------------------------
// This is the main file, it regulates the flow of the program, and the UI

#include <SFML/Graphics.hpp>

#include "pch.hpp"
// Precompiled headers this file uses:
// iostream, fstream, sstream
// cmath
// vector, string, tuple

#include "src/cmakevars.h"
#include "Camera.hpp"

//----------------------------------------------------------



int g_telescope_FOV;   //telescope_FOV in arcseconds

ObjectDatabase database;
Camera cam;


void WindowSetup(){
    //init window
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode(cam.window_w(), cam.window_h(), desktop.bitsPerPixel), database.name(), sf::Style::Default, settings);
    window.setFramerateLimit(60);

    sf::View view(sf::Vector2f(cam.raOffset(), cam.decOffset()), sf::Vector2f(cam.view_w(), cam.view_h()));
    view.rotate(180);

    sf::Font font; 
    if (!font.loadFromFile("resources/arial.ttf"))
        fmt::print("Font not found, using default font\n");

    sf::Text infoText;
    infoText.setFont(font);
    infoText.setCharacterSize(20);
    infoText.setRotation(180.f);

    sf::Text probText;
    probText.setFont(font);
    probText.setCharacterSize(20);
    probText.setRotation(180.f);
    
    sf::RectangleShape kvadrat(sf::Vector2f(g_telescope_FOV, g_telescope_FOV));
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
                else if (event.key.code == sf::Keyboard::C) database.clear_pictures();
                else if (event.key.code == sf::Keyboard::U) database.undo_picture();
                else if (event.key.code == sf::Keyboard::R) cam.reset_position(g_telescope_FOV, &database);
                else if (event.key.code == sf::Keyboard::H){
                    fmt::print("\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n\n",
                    "Left Click to add an observation target",
                    "Right Click to remove an observation target",
                    "Q to exit the window and confirm the selection",
                    "R to reset the view",
                    "U to undo an observation target",
                    "C to remove all observation targets",
                    "You can zoom in and out with the scroll wheel",
                    "Arrow keys for panning the view");
                }
            }
            else if(event.type == sf::Event::Resized){
                cam.change_dimensions(event.size.width, event.size.height);
                cam.reset_position(g_telescope_FOV, &database);
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
                    database.remove_picture(database.closest_picture_index(xd, yd));
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
            kvadrat.setPosition(mouseRa-g_telescope_FOV/2, mouseDec-g_telescope_FOV/2);
            window.draw(kvadrat);
        }

        // draw picture areas
        for(int i = 0; i < database.pictures.size(); i++){
            float xd, yd;
            std::tie(xd, yd) = database.pictures[i].offsets();

            // draw picture area shadow
            kvadrat.setPosition(xd-g_telescope_FOV/2, yd-g_telescope_FOV/2);
            kvadrat.setOutlineColor(sf::Color(100, 100, 100));
            kvadrat.setOutlineThickness(3.5f/cam.zoom());
            window.draw(kvadrat);

            // draw pictue area
            kvadrat.setOutlineColor(sf::Color(255, 255, 0));
            kvadrat.setOutlineThickness(2.f/cam.zoom());
            window.draw(kvadrat);
            
            // use probtext to print the name of the picture in the middle
            probText.setString(database.pictures[i].getName());
            probText.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
            probText.setFillColor(sf::Color(255, 255, 0));
            probText.setPosition(xd, yd);
            window.draw(probText);
        }

        //show which square will be deleted if the button is released
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !database.pictures.empty()){
            int ind = database.closest_picture_index(mouseRa, mouseDec);
            float xd, yd;
            std::tie(xd, yd) = database.pictures[ind].offsets();
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(mouseRa, mouseDec)), 
                sf::Vertex(sf::Vector2f(xd, yd))
            };
            window.draw(line, 2, sf::Lines);
        }

        //INFO TEXT:
        infoText.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
        infoText.setPosition(cam.raOffset()+cam.view_w()/2.f, cam.decOffset()+cam.view_h()/2.f);
        //the percentage that shows datapoints within the current cursor area
        std::string capturePercent = fmt::format("{:.2f}%", (float)database.ephemeris_in_picture(mouseRa, mouseDec)/database.obj_data.size()*100.f);
        //set the string to RA & DEC of the mouse and the capturePercent value
        infoText.setString(fmt::format("Offsets:\nRa: {:.2f}\nDec: {:.2f}\n\n{}", mouseRa, mouseDec, capturePercent));
        window.draw(infoText);


        //PROBABILITY TEXT:
        probText.setFillColor(sf::Color(255, 255, 255));
        probText.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
        probText.setPosition(cam.raOffset()-cam.view_w()*2.5f/7.f, cam.decOffset()+cam.view_h()/2.f);
        //total percentage of captured datapoints
        std::string per_pic = "", total = fmt::format("{:.2f}", database.selectedPercent());
        //percentage per picture
        for(int i = 0; i < database.pictures.size(); i++){
            float percent = 100.f * database.pictures[i].containedEphemeris() / database.obj_data.size();
            per_pic += fmt::format("{} {:.2f}%\n", database.pictures[i].getName(), percent);
        }
        //set the text
        probText.setString(fmt::format("{}\n= {}%", per_pic, total));
        window.draw(probText);

        
        window.display();
    }
}



int defaultVariables(){
    int W, H;
    std::string linija;

    std::ifstream ReadFile("data/variables.txt");
    if (!ReadFile.is_open()){
        fmt::print("Error: didnt find \"data/variable.txt\"\n");
        return 1;
    }
    while(getline(ReadFile, linija)){
        if (!linija.size()) continue;

        if (linija[0] == 'F'){
            std::string a = linija.substr(5, linija.length()-5);
            std::stringstream ss(a);
            ss >> g_telescope_FOV;
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
    return 0;
}

//args syntax: ./MPCS [-u|--url <str>] [-e|--exposition <int>] [-n|--number <int>] [-f|--fov <int>] [-c|--copy] [-x|--exit]
int main(int argc, char** argv){
    std::string version;
    #ifdef VERSION_MICRO
        version = fmt::format("Running MPCSolver {}.{}.{}\n\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    #else
        version = fmt::format("Running MPCSolver {}.{}\n\n", VERSION_MAJOR, VERSION_MINOR);
    #endif

    std::string obj_url = "";
    int pic_exposition=0, pic_number=0;
    bool to_clipboard = false, close_after = false;

    //Argument parser
    args::ArgumentParser parser("MPCS - Minor Planet Center Solver", version);
    args::HelpFlag help(parser, "help", "Display this message", {'h', "help"});
    args::ValueFlag<std::string> url(parser, "url", "the url to the object offsets link", {'u', "url"});
    args::ValueFlag<int> exposition(parser, "exposition", "the exposition duration (seconds)", {'e', "expositon"});
    args::ValueFlag<int> number(parser, "number", "number of pictures to be taken", {'n', "number"});
    args::ValueFlag<int> fov(parser, "fov", "fov of the telescope", {'f', "fov"});
    args::Flag copy(parser, "copy", "copy to clipboard", {'c', "copy"});
    args::Flag exit(parser, "exit", "exit the program after use", {'x', "exit"});

    try{
        parser.ParseCLI(argc, argv);
        fmt::print(version);
    } 
    catch (args::Help) {
        std::cout << std::endl << parser << std::endl;
        return 0;
    } 
    catch (args::ParseError e) {
        fmt::print("\n{}\n", e.what());
        std::cout << parser << std::endl;
        return 1;
    }
    catch (args::ValidationError e){
        fmt::print("\n{}\n", e.what());
        std::cout << parser << std::endl;
        return 1;
    }
    if (url) obj_url = args::get(url);
    if (exposition) pic_exposition = args::get(exposition);
    if (number) pic_number = args::get(number);
    if (fov) g_telescope_FOV = args::get(fov); 
    if (exit) close_after = true; 
    //if no parameters were passed, assume copy to be true
    if (copy || (!url && !exposition && !number && !fov && !exit)) to_clipboard = true; 

    if (defaultVariables()) return 1;

    while(true){
        database.reset();

        if (obj_url == "") fmt::print("Insert the website URL:\n");
        //this while loop is here to make sure a link is provided
        while(!obj_url.size()) std::getline(std::cin, obj_url);
        fmt::print("\n");
        
        int greska = database.fill_database(obj_url);
        if (greska){
            if (close_after) break;
            else{
                pic_exposition = pic_number = 0;
                obj_url = "";
                fmt::print("\n\n");
                continue;
            }
        }

        database.set_FOV(g_telescope_FOV);
        cam.reset_position(g_telescope_FOV, &database);
        
        fmt::print("\nObject: {}\n", database.name());

        if (!pic_number){
            fmt::print("Insert the ammount of pictures: ");
            std::cin >> pic_number;
        }
        database.set_ammount(pic_number);

        if (!pic_exposition){
            fmt::print("Insert the exposure length (in seconds): ");
            std::cin >> pic_exposition;
        }
        database.set_exposure(pic_exposition);

        WindowSetup();

        database.export_observation_targets(to_clipboard);
        
        if (close_after){
            if (!to_clipboard){
                fmt::print("Press enter to exit");
                std::cin.ignore(); std::cin.ignore();
                // two ignores because from some reason the program skips over the first one
            }
            break;
        }
        else{
            pic_exposition = pic_number = 0;
            obj_url = "";
            fmt::print("\n\n");
        }
    }
    return 0;
}
