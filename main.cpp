#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>
#include <cfloat>
#include <string>
#include <tuple>
#include <cmath>
#include <set>

int telescope_FOV;                                  //telescope_FOV in arcseconds
int W, H;                                           //window width and height
double centx, centy;                                //window observation center
double zoomFactor;                                  //window zoom factor
float sqdim;                                        //square and padding size in pixels

std::vector<std::pair<int, int> > db;               //database
std::vector<int> cate;                              //categorises offset types for assigning color
int minx, miny, maxx, maxy;                         //database max/min for x and y 
std::vector<std::pair<double, double> > squares;    //locations of observation targets
double zero_r, zero_d;                              //declination and right anscension of coordinates 0,0
std::string obj_context;                            //data about the observed object
std::string obj_name;                               //object name


//-----------------------CURL/PARSER STUFF-----------------------

//GenData creates a database of offsets
void GenData()
{
    using namespace std;

    //extract website data
    printf("Insert the website URL: ");
    string lynk;
    getline(cin, lynk);

    //using inbuilt windows curl command to dump website data to a text file (simpler than alternatives)
    string exec = "curl \"" + lynk + "\" --output zdb.txt";
    system(exec.c_str());

    //read the created txt file
    ifstream MyReadFile("zdb.txt");
    string linije, contxt;
    while(getline(MyReadFile, linije))
    {
        if (linije.size()){
            if (linije[0] == ' ')
            {
                if (db.empty()) contxt = linije;
                int len = 0;
                //isolate numbers from a string
                while(linije[len] != '<') len++;
                string linja = linije.substr(0, len);
                int stringResize = linja.length() - count(linja.begin(), linja.end(), ' ');
                remove(linja.begin(), linja.end(), ' ');
                linja.resize(stringResize);

                //insert numbers into the database
                bool frst = true;
                int x=0, y=0, negativ;
                for(int i = 0; i < linja.size(); i++)
                {
                    if (linja[i] == '+')
                    {
                        if (i!=0)
                        {
                            frst = false;
                            x*=negativ;
                        }
                        negativ = 1;
                    }
                    else if (linja[i] == '-')
                    {
                        if (i!=0)
                        {
                            frst = false;
                            x*=negativ;
                        }
                        negativ = -1;
                    }
                    else
                    {
                        if (frst) x=10*x+(linja[i]-'0');
                        else y=10*y+(linja[i]-'0');
                    }
                } y*=negativ;
                db.emplace_back(make_pair(x,y));

                //read offset type
                len = linije.size()-1;
                string tipe = "";
                while(linije[len] != '>')
                {
                    if (linije[len] != 'N')
                        tipe.push_back(linije[len]);
                    len--;
                }
                cate.emplace_back(tipe.length());
            }
            else if (linije.substr(0,4) == "<h1>")
            {
                int loc = 28;
                while (linije[loc] != ' ') loc++;
                obj_name = linije.substr(28, loc-28);
            }
        }
    }
    printf("\n");
    if(db.empty())
        printf("Link error: No data found, check if the link is right\n\n");

    //find max/min values for x and y
    minx = INT_MAX; miny = INT_MAX; maxx = INT_MIN; maxy = INT_MIN;
    for(int i = 0; i < db.size(); i++)
    {
        if (db[i].first > maxx) maxx = db[i].first;
        else if (db[i].first < minx) minx = db[i].first;
        if (db[i].second > maxy) maxy = db[i].second;
        else if (db[i].second < miny) miny = db[i].second;
    }

    //get object data from the +0 +0 orbit Ephemeris#1 link
    int p1=1, p2=contxt.length()-1;
    while(contxt[p1] != '"' && p1 < contxt.length()-1) p1++;
    while(contxt[p2] != '"' && p2 > 0) p2--;

    if (!contxt.length())
        printf("Link error: No object context found\n\n");

    lynk = contxt.substr(p1+1, p2-p1-2);
    exec = "curl \"" + lynk + "\" --output zcal.txt";
    system(exec.c_str());
    ifstream ReadFile("zcal.txt");
    bool fnd = false;
    while(getline(ReadFile, linije))
    {
        if (linije.size()){
        if (linije[0] == '2'){
            obj_context = linije;
            string rec = linije.substr(18, 10);
            string dec = linije.substr(29, 9);

            //convert coordinates to an int number
            int neg = 1; if (dec[0] == '-') neg = -1;
            zero_d = (((dec[1]-'0')*10+(dec[2]-'0'))*3600+((dec[4]-'0')*10+(dec[5]-'0'))*60+(dec[7]-'0')*10+(dec[8]-'0'))*neg;
            zero_r = (((rec[0]-'0')*10+(rec[1]-'0'))*3600+((rec[3]-'0')*10+(rec[4]-'0'))*60+(rec[6]-'0')*10+(rec[7]-'0')+(double)(rec[9]-'0')/10)*15;

            fnd = true;
            break;
        }
        }
    }
    if (!fnd)
        printf("Link error: No coordinate data found for the object\n\n");
    
    return;
}


//-----------------------CALC STUFF-----------------------

//function for converting db numbers to coordinates
std::tuple<double, double> db2Coords(int x, int y)
{
    using namespace std;

    double d = (double) max(maxx-minx, maxy-miny) / min(W, H);

    double x2c = (x-minx)/d;
    double y2c = (y-miny)/d;

    return make_tuple(x2c, y2c);
}

//coordinates on window, to absolute coordinates
std::tuple<double, double> Coords2Loc(double x, double y)
{
    using namespace std;

    x = W - x;
    y = H - y;

    double x2l = (x-(double)W/2)*zoomFactor + centx;
    double y2l = (y-(double)H/2)*zoomFactor + centy;

    return make_tuple(x2l, y2l);
}

//absolute coordinates to database numbers
std::tuple<double, double> Loc2db(double x, double y)
{
    using namespace std;
    double d = (double) max(maxx-minx, maxy-miny) / min(W, H);

    double x2d = d*x+minx;
    double y2d = d*y+miny;

    return make_tuple(x2d, y2d);
}

//square dimension fixer
void FixSqdim()
{
    using namespace std;

    float x1, y1;
    tie(x1, y1) = db2Coords(telescope_FOV, 0);
    float x2, y2;
    tie(x2, y2) = db2Coords(0, 0);
    
    sqdim = x1 - x2;
}

//recieves a db coordinate pair and returns the index of the closest square to it
int findClosest(double x, double y)
{
    double mind = DBL_MAX;
    int clsst;
    for (int i = 0; i < squares.size(); i++)
    {
        int x1 = squares[i].first, y1 = squares[i].second;
        double d = sqrt((long long) pow((long long)x-x1, 2) + pow((long long)y-y1, 2));
        if (d<mind)
        {
            mind = d;
            clsst = i;
        }
    }
    return clsst;
}

//-----------------------WINDOW STUFF-----------------------

void WindowSetup()
{
    using namespace sf;

    //init window
    RenderWindow window(VideoMode(W, H), obj_name);
    std::tie(centx, centy) = db2Coords(0, 0);
    View view(Vector2f(centx, centy), Vector2f(W, H));
    view.rotate(180);
    FixSqdim();

    Font font; font.loadFromFile("arial.ttf");
    Text text;
    text.setFont(font);
    text.setCharacterSize(20);
    text.setRotation(180.f);

    bool fokus = true;          //is window focused?
    bool mistu = false;         //is mouse on the window?
    double sqlx, sqly;          //where is the mouse?
    double csqx, csqy;          //where is the closest square?
    bool brisanje = false;      //is a square being deleted?
    while(window.isOpen())
    {
        //Event processing 
        Event event;
        while(window.pollEvent(event))
        {
            if(event.type == Event::Closed)
                window.close();
            
            if(event.type == Event::KeyPressed)
            {
                if (event.key.code == Keyboard::Q)
                    window.close();
                
                if (event.key.code == Keyboard::C)
                    squares.clear();
        
                if (event.key.code == Keyboard::U && !squares.empty())
                    squares.pop_back();
                
                if (event.key.code == Keyboard::R)
                {
                    zoomFactor = 1;
                    std::tie(centx, centy) = db2Coords(0, 0);
                    view.setSize(Vector2f(W, H));
                    view.setCenter(centx, centy);
                    FixSqdim();
                }
            }
            
            if(event.type == Event::Resized)
            {
                W = event.size.width;
                H = event.size.height;
                view.setSize(Vector2f(W*zoomFactor, H*zoomFactor));
                FixSqdim();
            }

            if (event.type == Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == Mouse::Left)
                {
                    double xd, yd;
                    std::tie(xd, yd) = Coords2Loc(event.mouseButton.x, event.mouseButton.y);
                    std::tie(xd, yd) = Loc2db(xd-sqdim/2, yd-sqdim/2);
                    squares.emplace_back(std::make_pair(xd, yd));
                }
                if (event.mouseButton.button == Mouse::Right && !squares.empty())
                    brisanje = true;
            }

            if (event.type == Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == Mouse::Right && brisanje)
                {
                    double xd, yd;
                    std::tie(xd, yd) = Coords2Loc(event.mouseButton.x, event.mouseButton.y);
                    std::tie(xd, yd) = Loc2db(xd-sqdim/2, yd-sqdim/2);
                    squares.erase(squares.begin()+findClosest(xd, yd));
                    brisanje = false;
                }
            }

            if (event.type == Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.wheel == Mouse::VerticalWheel)
                {
                    int dlt = event.mouseWheelScroll.delta;
                    zoomFactor += dlt*0.05f*zoomFactor;

                    //set new center
                    double xd, yd;
                    std::tie(xd, yd) = Coords2Loc(event.mouseWheelScroll.x, event.mouseWheelScroll.y);
                    centx += (centx-xd)*0.05f*dlt;
                    centy += (centy-yd)*0.05f*dlt;

                    view.setCenter(centx, centy);
                    view.setSize(Vector2f(W*zoomFactor, H*zoomFactor));
                    FixSqdim();
                }
            }

            if (event.type == Event::MouseMoved)
            {
                std::tie(sqlx, sqly) = Coords2Loc(event.mouseMove.x, event.mouseMove.y);
                //if right mouse button is pressed find the closest square and remember its location
                if (brisanje)
                {
                    double xd, yd;
                    std::tie(xd, yd) = Coords2Loc(event.mouseMove.x, event.mouseMove.y);
                    std::tie(xd, yd) = Loc2db(xd-sqdim/2, yd-sqdim/2);
                    int ind = findClosest(xd, yd);
                    std::tie(csqx, csqy) = db2Coords(squares[ind].first, squares[ind].second);
                }
            }

            if (event.type == Event::LostFocus)
                fokus = false;
            if (event.type == Event::GainedFocus)
                fokus = true;
            if (event.type == Event::MouseEntered)
                mistu = true;
            if (event.type == Event::MouseLeft)
                mistu = false;

            //h hide/show help gui
        }
        window.clear();
        window.setView(view);

        //draw dots
        CircleShape tocka(1.5f*zoomFactor);
        for(int i = 0; i < db.size(); i++)
        {
            if (cate[i] == 0) tocka.setFillColor(Color(0, 255, 0));
            else if (cate[i] == 2) tocka.setFillColor(Color(255, 255, 0));
            else if (cate[i] == 3) tocka.setFillColor(Color(255, 0, 0));
            else if (cate[i] == 4) tocka.setFillColor(Color(255, 255, 255));
            else if (cate[i] == 11) tocka.setFillColor(Color(0, 0, 255));
            else tocka.setFillColor(Color(255, 0, 255));

            double x, y;
            std::tie(x, y) = db2Coords(db[i].first, db[i].second);
            tocka.setPosition(x, y);
            window.draw(tocka);
        }

        //square projecting
        RectangleShape kvadrat(Vector2f(sqdim, sqdim));
        kvadrat.setFillColor(Color::Transparent);
        kvadrat.setOutlineThickness(zoomFactor);

        //draw a blue square on cursor location
        if (mistu && fokus)
        {
            kvadrat.setOutlineColor(Color(0, 255, 255));
            kvadrat.setPosition(sqlx-sqdim/2, sqly-sqdim/2);
            window.draw(kvadrat);
        }

        //draw other squares
        kvadrat.setOutlineColor(Color(255, 255, 0));
        for(int i = 0; i < squares.size(); i++)
        {
            double x,y;
            std::tie(x, y) = db2Coords(squares[i].first, squares[i].second);
            kvadrat.setPosition(x, y);
            window.draw(kvadrat);
        }

        //show which square will be deleted if the button is released
        if (brisanje)
        {
            sf::Vertex line[] = {Vertex(Vector2f(sqlx, sqly)), Vertex(Vector2f(csqx+sqdim/2, csqy+sqdim/2))};
            window.draw(line, 2, Lines);
        }

        //show text
        

        double xstr, ystr;
        std::tie(xstr, ystr) = Loc2db(sqlx, sqly);
        std::string xs = std::to_string(xstr);
        std::string ys = std::to_string(ystr);
        text.setString("Offsets:\nRA= " + xs + "\nDEC= " + ys);
        text.setScale(zoomFactor, zoomFactor);
        text.setPosition(centx+W/2*zoomFactor, centy+H/2*zoomFactor);
        window.draw(text);

        window.display();
    }
}


//-----------------------RESULT OUTPUT STUFF-----------------------

//add clipboard string (cpb) to clipboard
void toClipboard(std::string cpb)
{   
    const char* output = cpb.c_str();
    const size_t len = strlen(output) + 1;
    HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
    memcpy(GlobalLock(hMem), output, len);
    GlobalUnlock(hMem);
    OpenClipboard(0);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();

    printf("\nCopied to clipboard\n\n\n");
    return;
}

void observationTargets()
{
    using namespace std;
    string obj_pre = obj_context.substr(0, 18);
    string obj_post = obj_context.substr(38, obj_context.length()-38);
    string cpb=obj_name+"\n";

    //since the data cointained in post is unknown for a specific orbit, it is removed
    for(int i = 0; i < obj_post.length(); i++) if (obj_post[i] != ' ' && obj_post[i] != '.') obj_post[i] = 'x';

    printf("\nData:\n");

    //print out the targets
    for(int i = 0; i < squares.size(); i++)
    {
        double nr = (squares[i].first + zero_r)/15;
        double nd = squares[i].second + zero_d;
        string sgn = "+"; if (nd < 0) {sgn="-"; nd*=-1;}

        int r1 = floor(nr/3600);
        int r2 = floor(((nr/3600)-r1)*60);
        int r3 = floor(((((nr/3600)-r1)*60)-r2)*600);
        string pr1 = to_string(r1); while (pr1.size() < 2) pr1 = "0" + pr1;
        string pr2 = to_string(r2); while (pr2.size() < 2) pr2 = "0" + pr2;
        string pr3 = to_string(r3); while (pr3.size() < 3) pr3 = "0" + pr3;

        int d1 = floor(nd/3600);
        int d2 = floor(((nd/3600)-d1)*60);
        int d3 = floor(((((nd/3600)-d1)*60)-d2)*60);
        string pd1 = to_string(d1); while (pd1.size() < 2) pd1 = "0" + pd1;
        string pd2 = to_string(d2); while (pd2.size() < 2) pd2 = "0" + pd2;
        string pd3 = to_string(d3); while (pd3.size() < 2) pd3 = "0" + pd3;

        pr3.insert(2, ".");
        string rec = pr1 + " " + pr2 + " " + pr3;
        string dec = sgn + pd1 + " " + pd2 + " " + pd3;
        string tmps = obj_pre + rec + " " + dec + obj_post + "\n";
        printf (tmps.c_str());

        //add to clipboard string
        cpb += tmps;
    }

    if (cpb.length())
        toClipboard(cpb);

    return;
}


//-----------------------MAIN-----------------------

void defaultVariables()
{
    using namespace std;

    ifstream ReadFile("variables.txt");
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
            if (linija[0] == 'H') ss >> H;
            else if (linija[0] == 'W') ss >> W;
        }
        }
    }
}

int main(void)
{
    while(true)
    {
        db.clear();
        squares.clear();
        zoomFactor = 1;
        defaultVariables();

        GenData();
        
        printf("\nOBJECT: ");
        printf(obj_name.c_str());
        printf("\n");

        WindowSetup();
        observationTargets();

        system("pause");
        printf("\n\n");
    }
    return 0;
}