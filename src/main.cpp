//----------------------------------------------------------

#include "pch.hpp"

#include <SFML/Graphics.hpp>
#include <inipp/inipp.h>
#include <args.hxx>

#include "src/cmakevars.h"
#include "Camera.hpp"

//----------------------------------------------------------



void WindowSetup(ObjectDatabase& database, Camera& cam)
{
    //init window
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    auto [window_W, window_H] = cam.getWindowSize();
    sf::RenderWindow window(sf::VideoMode(window_W, window_H, desktop.bitsPerPixel), database.name(), sf::Style::Default, settings);
    window.setFramerateLimit(60);

    // the dimensions of the view window
    auto [view_W, view_H] = cam.getView();
    
    sf::View view(sf::Vector2f(cam.raOffset(), cam.decOffset()), sf::Vector2f(view_W, view_H));
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
    
    sf::RectangleShape kvadrat(sf::Vector2f(database.getFOV(), database.getFOV()));
    kvadrat.setFillColor(sf::Color::Transparent);

    bool fokus = true;                      //is window focused?
    float mouseRa = 0.f, mouseDec = 0.f;    //where is the mouse?
    while(window.isOpen()){
        //Event processing 
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed) window.close();
            else if(event.type == sf::Event::KeyPressed){
                if (event.key.code == sf::Keyboard::Q) window.close();
                else if (event.key.code == sf::Keyboard::C) database.clear_pictures();
                else if (event.key.code == sf::Keyboard::U) database.undo_picture();
                else if (event.key.code == sf::Keyboard::R) cam.reset_position(database.getFOV(), database);
                else if (event.key.code == sf::Keyboard::H)
                    fmt::print(
                        "\nLeft Click to add an observation target\n"
                        "Right Click to remove an observation target\n"
                        "Q to exit the window and confirm the selection\n"
                        "R to reset the view\n"
                        "U to undo an observation target\n"
                        "C to remove all observation targets\n"
                        "You can zoom in and out with the scroll wheel\n"
                        "Arrow keys for panning the view\n\n"
                    );
            }
            else if(event.type == sf::Event::Resized){
                cam.setDimensions(event.size.width, event.size.height);
                cam.reset_position(database.getFOV(), database);
            }
            else if (event.type == sf::Event::MouseButtonPressed){
                if (event.mouseButton.button == sf::Mouse::Left){
                    auto [xd, yd] = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
                    database.insert_picture(xd, yd);
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased){
                if (event.mouseButton.button == sf::Mouse::Right){
                    auto [xd, yd] = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
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
        std::tie(view_W, view_H) = cam.getView(); // tie variables since their calls do computation
        view.setSize(sf::Vector2f(view_W, view_H));
        view.setCenter(cam.raOffset(), cam.decOffset());
        window.setView(view);

        sf::Vector2i pos = sf::Mouse::getPosition(window);
        std::tie(mouseRa, mouseDec) = cam.px_to_off(pos.x, pos.y);

        //draw dots
        sf::CircleShape tocka(1.5f/cam.getZoom());
        for(int i = 0; i < database.getEphAm(); i++)
        {
            auto [R, G, B] = database.getEph(i).getColor();
            tocka.setFillColor(sf::Color(R, G, B));
            auto [x, y] = database.getEph(i).getOffsets();
            tocka.setPosition(x, y);
            window.draw(tocka);
        }

        //setup the square projection settings
        kvadrat.setOutlineThickness(2.f/cam.getZoom());

        //draw a blue square on cursor location
        if (fokus)
        {
            kvadrat.setOutlineColor(sf::Color(0, 255, 255));
            kvadrat.setPosition(mouseRa-database.getFOV()/2, mouseDec-database.getFOV()/2);
            window.draw(kvadrat);
        }

        // draw picture areas
        for(int i = 0; i < database.getPicAm(); i++)
        {
            auto [xd, yd] = database.getPic(i).getOffsets();

            // draw picture area shadow
            kvadrat.setPosition(xd-database.getFOV()/2, yd-database.getFOV()/2);
            kvadrat.setOutlineColor(sf::Color(100, 100, 100));
            kvadrat.setOutlineThickness(3.5f/cam.getZoom());
            window.draw(kvadrat);

            // draw pictue area
            kvadrat.setOutlineColor(sf::Color(255, 255, 0));
            kvadrat.setOutlineThickness(2.f/cam.getZoom());
            window.draw(kvadrat);
            
            // use probtext to print the name of the picture in the middle
            probText.setString(database.getPic(i).getName());
            probText.setScale(1.f/cam.getZoom(), 1.f/cam.getZoom());
            probText.setFillColor(sf::Color(255, 255, 0));
            probText.setPosition(xd, yd);
            window.draw(probText);
        }

        //show which square will be deleted if the button is released
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !database.getPicAm() != 0)
        {
            int ind = database.closest_picture_index(mouseRa, mouseDec);
            auto [xd, yd] = database.getPic(ind).getOffsets();
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(mouseRa, mouseDec)), 
                sf::Vertex(sf::Vector2f(xd, yd))
            };
            window.draw(line, 2, sf::Lines);
        }

        //INFO TEXT:
        infoText.setScale(1.f/cam.getZoom(), 1.f/cam.getZoom());
        infoText.setPosition(cam.raOffset()+view_W/2.f, cam.decOffset()+view_H/2.f);
        //the percentage that shows datapoints within the current cursor area
        std::string capturePercent = fmt::format("{:.2f}%", (float)database.ephemeris_in_picture(mouseRa, mouseDec)/database.getEphAm()*100.f);
        //set the string to RA & DEC of the mouse and the capturePercent value
        infoText.setString(fmt::format("Offsets:\nRa: {:.2f}\nDec: {:.2f}\n\n{}", mouseRa, mouseDec, capturePercent));
        window.draw(infoText);


        //PROBABILITY TEXT:
        probText.setFillColor(sf::Color(255, 255, 255));
        probText.setScale(1.f/cam.getZoom(), 1.f/cam.getZoom());
        probText.setPosition(cam.raOffset()-view_W*2.5f/7.f, cam.decOffset()+view_H/2.f);
        //total percentage of captured datapoints
        std::string per_pic = "", total = fmt::format("{:.2f}", database.calculateSelected());
        //percentage per picture
        for(int i = 0; i < database.getPicAm(); i++){
            float percent = 100.f * database.getPic(i).getContainedEphemeris() / database.getEphAm();
            per_pic += fmt::format("{} {:.2f}%\n", database.getPic(i).getName(), percent);
        }
        //set the text
        probText.setString(fmt::format("{}\n= {}%", per_pic, total));
        window.draw(probText);

        
        window.display();
    }
}


// this function parses the MPCS.ini file
// \param[out] W window width 
// \param[out] H window height 
// \param[out] FOV telescope FOV
void defaultVariables(unsigned int& W, unsigned int& H, unsigned int& FOV)
{
    std::ifstream ReadFile("MPCS.ini");
    if (!ReadFile.is_open())
        fmt::print(
            "Warning: MPCS.ini does not exist, or isnt in the right directory!\n\n"
        );

    // initialize inipp
    inipp::Ini<char> ini;

    // inipp magic
    ini.parse(ReadFile);
    if (!inipp::get_value(ini.sections["Window"], "W", W)){
        W = 1080;
        fmt::print(
            "Warning: Window width not properly specified in MPCS.ini!\n"
            "Defaulting to W = {}\n", W
        );
    }
    if (!inipp::get_value(ini.sections["Window"], "H", H)){
        H = 920;
        fmt::print(
            "Warning: Window height not properly specified in MPCS.ini!\n"
            "Defaulting to H = {}\n", H
        );
    }
    if (!inipp::get_value(ini.sections["Telescope"], "FOV", FOV)){
        FOV = 2500;
        fmt::print(
            "Warning: Telescope FOV not properly specified in MPCS.ini!\n"
            "Defaulting to FOV = {}\n", FOV
        );
    }

    fmt::print("\n");
    ReadFile.close();
}


// args syntax: ./MPCS [-u|--url <str>] [-e|--exposition <int>] [-n|--number <int>] [-c|--copy] [-x|--exit]
int main(int argc, char** argv)
{
    // string containing the version of the MPCS
    std::string version;
    #ifdef MPCS_VERSION_MICRO
        version = fmt::format("MPCSolver {}.{}.{}", MPCS_VERSION_MAJOR, MPCS_VERSION_MINOR, MPCS_VERSION_MICRO);
    #else
        version = fmt::format("MPCSolver {}.{}", MPCS_VERSION_MAJOR, MPCS_VERSION_MINOR);
    #endif

    std::string obj_url = "";
    int pic_exposition=0, pic_number=0;
    bool to_clipboard = false, close_after = false;

    // Argument parser
    args::ArgumentParser parser("MPCS - Minor Planet Center Solver", version + '\n');
    // arguments
    args::HelpFlag help(parser, "help", "Display this message", {'h', "help"});
    args::ValueFlag<std::string> url(parser, "url", "the url to the object offsets link", {'u', "url"});
    args::ValueFlag<int> exposition(parser, "exposition", "the exposition duration (seconds)", {'e', "expositon"});
    args::ValueFlag<int> number(parser, "number", "number of pictures to be taken", {'n', "number"});
    args::Flag copy(parser, "copy", "copy to clipboard", {'c', "copy"});
    args::Flag exit(parser, "exit", "exit the program after use", {'x', "exit"});

    // try parsing arguments
    try{
        parser.ParseCLI(argc, argv);
        fmt::print("{}\n", version);
    } 
    // if parsing fails, print help message and inform user of the error
    catch (args::Help) {
        std::cout << std::endl << parser << std::endl;
        return 0;
    }
    catch (args::ParseError e) {
        fmt::print("\nError: {}\n\n", e.what());
        std::cout << parser << std::endl;
        return 1;
    }
    catch (args::ValidationError e){
        fmt::print("\nError: {}\n\n", e.what());
        std::cout << parser << std::endl;
        return 1;
    }

    // if parsing was successful store the inputed arguments
    if (url) obj_url = args::get(url);
    if (exposition) pic_exposition = args::get(exposition);
    if (number) pic_number = args::get(number);
    if (exit) close_after = true; 
    // if no parameters were passed, assume to_clipboard to be true
    if (copy || (!url && !exposition && !number && !exit)) to_clipboard = true; 

    // create the camera and the database
    ObjectDatabase database;
    Camera cam;

    // read the default variables
    try{
        unsigned int W, H, FOV;
        defaultVariables(W, H, FOV);
        cam.setDimensions(W, H);
        database.set_FOV(FOV);
    }
    catch (std::exception& e){
        fmt::print("Error: {} \n\n", e.what());
        return 1;
    }

    while(true)
    {
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
                database.reset();
                fmt::print("\n\n");
                continue;
            }
        }

        cam.reset_position(database.getFOV(), database);
        
        fmt::print("\nObject: {}\n", database.name());

        if (!pic_number){
            fmt::print("Insert the ammount of pictures: ");
            std::cin >> pic_number;
        }
        database.set_amount(pic_number);

        if (!pic_exposition){
            fmt::print("Insert the exposure length (in seconds): ");
            std::cin >> pic_exposition;
        }
        database.set_exposure(pic_exposition);

        WindowSetup(database, cam);

        database.export_observation_targets(to_clipboard);
        
        if (close_after){
            if (!to_clipboard){
                fmt::print("Press enter to exit");
                std::cin.ignore(); std::cin.ignore();
                // two ignores because from some reason the program sometimes skips over the first one
            }
            break;
        }
        else{
            pic_exposition = pic_number = 0;
            obj_url = "";
            database.reset();
            fmt::print("\n\n");
        }
    }
    return 0;
}
