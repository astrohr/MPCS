//----------------------------------------------------------

#include "pch.hpp"

#include <SFML/Graphics.hpp>
#include <inipp/inipp.h>
#include <args.hxx>

#include "src/cmakevars.h"
#include "Camera.hpp"

//----------------------------------------------------------

void WindowSetup(ObjectDatabase &database, Camera &cam)
{
    // init window
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
        log.wrn("Font not found, using default font\n");

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

    bool fokus = true;                   // is window focused?
    float mouseRa = 0.f, mouseDec = 0.f; // where is the mouse?
    while (window.isOpen())
    {
        // Event processing
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Q)
                    window.close();
                else if (event.key.code == sf::Keyboard::C)
                    database.clear_pictures();
                else if (event.key.code == sf::Keyboard::U)
                    database.undo_picture();
                else if (event.key.code == sf::Keyboard::R)
                    cam.reset_position(database.getFOV(), database);
                else if (event.key.code == sf::Keyboard::H)
                    log.msg(
                        "\nLeft Click to add an observation target\n"
                        "Right Click to remove an observation target\n"
                        "Q to exit the window and confirm the selection\n"
                        "R to reset the view\n"
                        "U to undo an observation target\n"
                        "C to remove all observation targets\n"
                        "You can zoom in and out with the scroll wheel\n"
                        "Arrow keys for panning the view\n\n");
            }
            else if (event.type == sf::Event::Resized)
            {
                cam.setDimensions(event.size.width, event.size.height);
                cam.reset_position(database.getFOV(), database);
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    auto [xd, yd] = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
                    database.insert_picture(xd, yd);
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Right)
                {
                    auto [xd, yd] = cam.px_to_off(event.mouseButton.x, event.mouseButton.y);
                    database.remove_picture(database.closest_picture_index(xd, yd));
                }
            }
            else if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                {
                    cam.change_zoom(event.mouseWheelScroll.delta, event.mouseWheelScroll.x, event.mouseWheelScroll.y);
                }
            }
            else if (event.type == sf::Event::LostFocus)
                fokus = false;
            else if (event.type == sf::Event::GainedFocus)
                fokus = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            cam.pan_camera(-1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            cam.pan_camera(0, -1);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            cam.pan_camera(1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            cam.pan_camera(0, 1);

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
        text.setScale(1.f / cam.getZoom(), 1.f / cam.getZoom());
        kvadrat.setOutlineThickness(2.f / cam.getZoom());
        tocka.setRadius(1.5f / cam.getZoom());

        // draw dots
        for (auto eph : database.getEphs())
        {
            auto [R, G, B] = eph.getColor();
            tocka.setFillColor(sf::Color(R, G, B));
            auto [x, y] = eph.getOffsets();
            tocka.setPosition(x, y);
            window.draw(tocka);
        }

        // draw a blue square on cursor location
        if (fokus)
        {
            kvadrat.setOutlineColor(sf::Color(0, 255, 255));
            kvadrat.setPosition(mouseRa - database.getFOV() / 2, mouseDec - database.getFOV() / 2);
            window.draw(kvadrat);
        }

        // draw picture areas
        for (auto pic : database.getPics())
        {
            auto [xd, yd] = pic.getOffsets();
            kvadrat.setPosition(xd - database.getFOV() / 2, yd - database.getFOV() / 2);

            // draw picture area shadow
            kvadrat.setOutlineColor(sf::Color(100, 100, 100));
            kvadrat.setOutlineThickness(3.5f / cam.getZoom());
            window.draw(kvadrat);

            // draw pictue area
            kvadrat.setOutlineColor(sf::Color(255, 255, 0));
            kvadrat.setOutlineThickness(2.f / cam.getZoom());
            window.draw(kvadrat);

            // use text to print the name of the picture in the middle
            text.setString(pic.getName());
            text.setFillColor(sf::Color(255, 255, 0));
            text.setPosition(xd, yd);
            window.draw(text);
        }

        // show which square will be deleted if the button is released
        if (sf::Mouse::isButtonPressed(sf::Mouse::Right) && !database.getPics().empty())
        {
            int ind = database.closest_picture_index(mouseRa, mouseDec);
            auto [xd, yd] = database.getPics()[ind].getOffsets();
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(mouseRa, mouseDec)),
                sf::Vertex(sf::Vector2f(xd, yd))};
            window.draw(line, 2, sf::Lines);
        }

        // PROBABILITY TEXT:
        text.setFillColor(sf::Color(255, 255, 255));
        text.setPosition(cam.raOffset() - view_W * 2.5f / 7.f, cam.decOffset() + view_H / 2.f);
        // total percentage of captured datapoints
        std::string per_pic = "", total = fmt::format("{:.2f}", database.calculateSelected());
        // percentage per picture
        for (auto pic : database.getPics())
        {
            float percent = 100.f * pic.getContainedEphemeris() / database.getEphs().size();
            per_pic += fmt::format("{} {:.2f}%\n", pic.getName(), percent);
        }
        // set the text
        text.setString(fmt::format("{}\n= {}%", per_pic, total));
        window.draw(text);

        // INFO TEXT:
        text.setPosition(cam.raOffset() + view_W / 2.f, cam.decOffset() + view_H / 2.f);
        // the percentage that shows datapoints within the current cursor area
        std::string capturePercent = fmt::format("{:.2f}%", database.ephemeris_in_picture(mouseRa, mouseDec) * 100.f / database.getEphs().size());
        // set the string to RA & DEC of the mouse and the capturePercent value
        text.setString(fmt::format("Offsets:\nRa: {:.2f}\nDec: {:.2f}\n\n{}", mouseRa, mouseDec, capturePercent));
        window.draw(text);

        window.display();
    }
}

// this function parses the MPCS.ini file
// \param[out] W window width
// \param[out] H window height
// \param[out] FOV telescope FOV
void defaultVariables(unsigned int &W, unsigned int &H, unsigned int &FOV)
{
    std::ifstream ReadFile("../resources/MPCS.ini");
    if (!ReadFile.is_open())
    {
        ReadFile.open("./resources/MPCS.ini");
        if (!ReadFile.is_open())
        {
            log.wrn(
                "MPCS.ini does not exist, or isnt in the right directory!\n\n");
        }
    }
    // initialize inipp
    inipp::Ini<char> ini;

    // inipp magic
    ini.parse(ReadFile);
    if (!inipp::get_value(ini.sections["Window"], "W", W))
    {
        W = 1080;
        log.wrn(
            std::format("Window width not properly specified in MPCS.ini!\nDefaulting to W = {}\n"
                        "",
                        W));
    }
    if (!inipp::get_value(ini.sections["Window"], "H", H))
    {
        H = 920;
        log.wrn(
            std::format("Window height not properly specified in MPCS.ini!\nDefaulting to H = {}\n",
                        H));
    }
    if (!inipp::get_value(ini.sections["Telescope"], "FOV", FOV))
    {
        FOV = 2500;
        log.wrn(
            std::format("Warning: Telescope FOV not properly specified in MPCS.ini!\nDefaulting to FOV = {}\n",
                        FOV));
    }

    std::cout << "\n";
    ReadFile.close();
}

// args syntax: ./MPCS [-u|--url <str>] [-e|--exposition <int>] [-n|--number <int>] [-c|--copy] [-x|--exit] [-f|--fov <int>]
int main(int argc, char **argv)
{
    // string containing the version of the MPCS
    std::string version;
#ifdef MPCS_VERSION_MICRO
    version = fmt::format("MPCSolver {}.{}.{}", MPCS_VERSION_MAJOR, MPCS_VERSION_MINOR, MPCS_VERSION_MICRO);
#else
    version = fmt::format("MPCSolver {}.{}", MPCS_VERSION_MAJOR, MPCS_VERSION_MINOR);
#endif

    // create the camera and the database
    ObjectDatabase database;
    Camera cam;

    // read the default variables
    try
    {
        unsigned int W, H, FOV;
        defaultVariables(W, H, FOV);
        cam.setDimensions(W, H);
        database.set_FOV(FOV);
    }
    catch (std::exception &e)
    {
        log.err(std::format("{} \n\n", e.what()));
        return 1;
    }

    std::string obj_url = "";
    int pic_exposition = 0, pic_number = 0;
    bool to_clipboard = false, close_after = false;

    // Argument parser
    args::ArgumentParser parser("MPCS - Minor Planet Center Solver", version + '\n');
    // arguments
    args::HelpFlag help(parser, "help", "Display this message", {'h', "help"});
    args::ValueFlag<std::string> url(parser, "url", "the url to the object offsets link", {'u', "url"});
    args::ValueFlag<int> exposition(parser, "exposition", "the exposition duration (seconds)", {'e', "expositon"});
    args::ValueFlag<int> number(parser, "number", "number of pictures to be taken", {'n', "number"});
    args::ValueFlag<int> fov(parser, "FOV", "telescope FOV", {'f', "fov"});
    args::Flag copy(parser, "copy", "copy to clipboard", {'c', "copy"});
    args::Flag exit(parser, "exit", "exit the program after use", {'x', "exit"});

    // try parsing arguments
    try
    {
        parser.ParseCLI(argc, argv);
        log.msg(std::format("{}\n", version));
    }
    // if parsing fails, print help message and inform user of the error
    catch (args::Help)
    {
        std::cout << std::endl
                  << parser << std::endl;
        return 0;
    }
    catch (args::ParseError e)
    {
        log.err(std::format("\n{}\n\n", e.what()));
        std::cout << parser << std::endl;
        return 1;
    }
    catch (args::ValidationError e)
    {
        log.err(std::format("\n{}\n\n", e.what()));
        std::cout << parser << std::endl;
        return 1;
    }

    // if parsing was successful store the inputed arguments
    if (url)
        obj_url = args::get(url);
    if (exposition)
        pic_exposition = args::get(exposition);
    if (number)
        pic_number = args::get(number);
    if (fov)
    {
        unsigned int FOV = args::get(fov);
        log.msg(std::format("FOV at {}\n", FOV));
        database.set_FOV(FOV);
    }
    if (exit)
        close_after = true;
    // if no parameters were passed, assume to_clipboard to be true
    if (copy || (!url && !exposition && !number && !exit))
        to_clipboard = true;

    while (true)
    {
        if (obj_url == "")
            std::cout << "Insert the website URL:\n";
        // this while loop is here to make sure a link is provided
        while (!obj_url.size())
            std::getline(std::cin, obj_url);
        std::cout << "\n";

        int greska = database.fill_database(obj_url);
        if (greska)
        {
            bool retry = false;
            if (greska == 1)
            {
                log.wrn("Link interaction failed, retry? (y/n): ");
                std::string s;
                std::cin >> s;
                if (!s.empty() && (s[0] == 'y' || s[0] == 'Y'))
                    retry = true;
            }

            if (close_after && !retry)
                break;
            else
            {
                if (!retry)
                    obj_url = "";
                pic_exposition = pic_number = 0;
                database.reset();
                std::cout << "\n\n";
                continue;
            }
        }

        cam.reset_position(database.getFOV(), database);

        log.msg(std::format("\nObject: {}\n", database.name()));

        if (!pic_number)
        {
            std::cout << "Insert the ammount of pictures: ";
            std::cin >> pic_number;
        }
        database.set_amount(pic_number);

        if (!pic_exposition)
        {
            std::cout << "Insert the exposure length (in seconds): ";
            std::cin >> pic_exposition;
        }
        database.set_exposure(pic_exposition);

        WindowSetup(database, cam);

        database.export_observation_targets(to_clipboard);

        if (close_after)
        {
            if (!to_clipboard)
            {
                log.msg("Press enter to exit");
                std::cin.ignore();
            }
            break;
        }
        else
        {
            pic_exposition = pic_number = 0;
            obj_url = "";
            database.reset();
            std::cout << "\n\n";
        }
    }
    return 0;
}
