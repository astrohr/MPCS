#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cfloat>
#include <string>
#include <tuple>
#include <cmath>

#include "cmakevars.h"
#include "Camera.hpp"

int g_telescope_FOV;   //telescope_FOV in arcseconds

ObjectDatabase database;
Camera cam;

//-----------------------WINDOW STUFF-----------------------

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
        std::cout << "Font not found, using default font" << std::endl;

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
        std::string total = std::to_string(database.selectedPercent());
        total = total.substr(0, total.size()-4);
        probText.setString(buraz + "\n= " + total + "%");
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

    std::ifstream LinksFile("data/allowed_links.txt");
    if (!LinksFile.is_open()){
        std::cout << "Error: didnt find \"data/allowed_links.txt\"" << std::endl;
        return 1;
    }
    while(getline(LinksFile, linija)){
        if(!linija.size()) continue;
        g_allowed_links.emplace_back(linija);
    }
    LinksFile.close();
    return 0;
}

int main(int argc, char** argv){
    //argc syntax: MPCS <url:str> <exposition:int> <number:int> <copy to clipboard:bool(1/0)> <FOV:int>
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

        if (argc == 6) g_telescope_FOV = atoi(argv[5]);

        database.set_FOV(g_telescope_FOV);
        cam.reset_position(g_telescope_FOV, &database);
        
        if (argc == 1) std::cout << "Object: " << database.name() << std::endl;

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
        if (argc > 4) copy_to_clipboard = atoi(argv[4]);
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
// create object class so that object database is simpler
// add precompiled headers and make classes independent