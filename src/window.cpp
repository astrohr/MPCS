//----------------------------------------------------------

#pragma once

#include "window.hpp"

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
    // dont worry that this will end up throwing a warning (sometimes)
    if (!font.loadFromFile("../resources/arial.ttf") && !font.loadFromFile("resources/arial.ttf"))
        fmt::print("Font not found, using default font\n");


    // text object for rendering text
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setRotation(180.f);
    
    // rectangle object for drawing all rectangles
    sf::RectangleShape kvadrat(sf::Vector2f(database.getFOV(), database.getFOV()));
    kvadrat.setFillColor(sf::Color::Transparent);

    // circle shape for drawing all dots
    sf::CircleShape tocka;

    bool fokus = true;                      //is window focused?
    float mouseRa = 0.f, mouseDec = 0.f;    //where is the mouse?
    while(window.isOpen())
    {
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



        // update the situation to represent potential input changes
        window.clear();
        std::tie(view_W, view_H) = cam.getView(); // tie variables since their calls do computation
        view.setSize(sf::Vector2f(view_W, view_H));
        view.setCenter(cam.raOffset(), cam.decOffset());
        window.setView(view);

        // remember mouse position
        sf::Vector2i pos = sf::Mouse::getPosition(window);
        std::tie(mouseRa, mouseDec) = cam.px_to_off(pos.x, pos.y);

        // set sizes
        text.setScale(1.f/cam.getZoom(), 1.f/cam.getZoom());
        kvadrat.setOutlineThickness(2.f/cam.getZoom());
        tocka.setRadius(1.5f/cam.getZoom());


        //draw dots
        for(auto eph : database.getEphs())
        {
            auto [R, G, B] = eph.getColor();
            tocka.setFillColor(sf::Color(R, G, B));
            auto [x, y] = eph.getOffsets();
            tocka.setPosition(x, y);
            window.draw(tocka);
        }

        //draw a blue square on cursor location
        if (fokus)
        {
            kvadrat.setOutlineColor(sf::Color(0, 255, 255));
            kvadrat.setPosition(mouseRa-database.getFOV()/2, mouseDec-database.getFOV()/2);
            window.draw(kvadrat);
        }

        // draw picture areas
        for(auto pic : database.getPics())
        {
            auto [xd, yd] = pic.getOffsets();
            kvadrat.setPosition(xd-database.getFOV()/2, yd-database.getFOV()/2);

            // draw picture area shadow
            kvadrat.setOutlineColor(sf::Color(100, 100, 100));
            kvadrat.setOutlineThickness(3.5f/cam.getZoom());
            window.draw(kvadrat);

            // draw pictue area
            kvadrat.setOutlineColor(sf::Color(255, 255, 0));
            kvadrat.setOutlineThickness(2.f/cam.getZoom());
            window.draw(kvadrat);
            
            // use text to print the name of the picture in the middle
            text.setString(pic.getName());
            text.setFillColor(sf::Color(255, 255, 0));
            text.setPosition(xd, yd);
            window.draw(text);
        }

        //show which square will be deleted if the button is released
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !database.getPics().empty())
        {
            int ind = database.closest_picture_index(mouseRa, mouseDec);
            auto [xd, yd] = database.getPics()[ind].getOffsets();
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(mouseRa, mouseDec)), 
                sf::Vertex(sf::Vector2f(xd, yd))
            };
            window.draw(line, 2, sf::Lines);
        }


        //PROBABILITY TEXT:
        text.setFillColor(sf::Color(255, 255, 255));
        text.setPosition(cam.raOffset()-view_W*2.5f/7.f, cam.decOffset()+view_H/2.f);
        //total percentage of captured datapoints
        std::string per_pic = "", total = fmt::format("{:.2f}", database.calculateSelected());
        //percentage per picture
        for(auto pic : database.getPics())
        {
            float percent = 100.f * pic.getContainedEphemeris() / database.getEphs().size();
            per_pic += fmt::format("{} {:.2f}%\n", pic.getName(), percent);
        }
        //set the text
        text.setString(fmt::format("{}\n= {}%", per_pic, total));
        window.draw(text);

        //INFO TEXT:
        text.setPosition(cam.raOffset()+view_W/2.f, cam.decOffset()+view_H/2.f);
        //the percentage that shows datapoints within the current cursor area
        std::string capturePercent = fmt::format("{:.2f}%", database.ephemeris_in_picture(mouseRa, mouseDec)*100.f/database.getEphs().size());
        //set the string to RA & DEC of the mouse and the capturePercent value
        text.setString(fmt::format("Offsets:\nRa: {:.2f}\nDec: {:.2f}\n\n{}", mouseRa, mouseDec, capturePercent));
        window.draw(text);

        
        window.display();
    }
}

