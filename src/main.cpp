// main.cpp ------------------------------------------------
// This is the main file, it regulates the flow of the program, and the UI

#include <SFML/Graphics.hpp>

#include "pch.hpp"
// Precompiled headers this file uses:
// iostream, fstream, sstream
// cfloat, cmath
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

        //draw other squares
        for(int i = 0; i < database.pictures.size(); i++){
            float xd, yd;
            std::tie(xd, yd) = database.pictures[i].offsets();
            kvadrat.setPosition(xd-g_telescope_FOV/2, yd-g_telescope_FOV/2);
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
        probText.setScale(1.f/cam.zoom(), 1.f/cam.zoom());
        probText.setPosition(cam.raOffset()-cam.view_w()*2.5f/7.f, cam.decOffset()+cam.view_h()/2.f);
        //total percentage of captured datapoints
        std::string per_pic = "", total = fmt::format("{:.2f}", database.selectedPercent());
        //percentage per picture
        for(int i = 0; i < database.pictures.size(); i++)
            per_pic += fmt::format("{} {}\n", database.pictures[i].sign(), database.pictures[i].percentStr(database.obj_data.size()));
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

int main(int argc, char** argv){
    //argc syntax: MPCS <url:str> <exposition:int> <number:int> <copy to clipboard:bool(1/0)> <FOV:int>
    //i wont check your inputs, make sure you code them right yourself
    if (argc > 6){
        fmt::print("Error: Too many arguments\n");
        return 0;
    }
    fmt::print("Running MPCSolver {}.{}\n\n", VERSION_MAJOR, VERSION_MINOR);

    if (defaultVariables()) return 1;
    while(true){
        database.reset();

        std::string url;
        if (argc == 1){
            fmt::print("Insert the website URL:\n");
            //the for is here to make sure that the url inserted isnt an empty line
            for(int i = 0; i < 3 && !url.size(); i++) std::getline(std::cin, url);
            fmt::print("\n");
        }
        else url = argv[1];
        int greska = database.fill_database(url);
        if (greska){
            fmt::print("\n\n");
            if (argc == 1) continue;
            else break;
        }

        if (argc == 6) g_telescope_FOV = atoi(argv[5]);

        database.set_FOV(g_telescope_FOV);
        cam.reset_position(g_telescope_FOV, &database);
        
        if (argc == 1) fmt::print("\nObject: {}\n", database.name());

        int amm, exp;
        if (argc < 3){
            fmt::print("Insert the ammount of pictures: ");
            std::cin >> amm;
        }
        else amm = atoi(argv[2]);
        database.set_ammount(amm);

        if (argc < 4){
            fmt::print("Insert the exposure length (in seconds): ");
            std::cin >> exp;
        }
        else exp = atoi(argv[3]);
        database.set_exposure(exp);

        WindowSetup();

        bool copy_to_clipboard = true;
        if (argc > 4) copy_to_clipboard = atoi(argv[4]);
        database.export_observation_targets(copy_to_clipboard);
        
        if (argc > 1){
            fmt::print("Click any key to exit...\n");
            std::cin.get();
            break; // if the program was called from the console just kill it after use
        }
        fmt::print("\n\n");
    }
    return 0;
}

//TODO:
// Try to make an algorythm for automatically selecting squares in an optimal manner
//  - linear regression is possible in most cases (detection algorythm needed), in others its NP (heuristic?)
// Make ephemeris save their positons change trough time
// Implement time passing display (and linear approximations for movements between hours)
// Add object selection menu
// Remove the console aspect of the app
// Show the coordinate system
// do some inheritance for camera, picture and ephemeris
// create object class so that object database is simpler
// make classes more independent from submodules
//  - ephemeris.hpp does not need sfml/color since you can just save RGB values in a tuple or hex or something (check how sfml does it)
// You gotta comment the code a bit better man
// replace data folder with a .ini file