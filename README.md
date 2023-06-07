# MPCS
A program for solving the NEO uncertainty

MPCS stands for "Minor Planet Center Solver"

## Requirements for building
Cmake version 3.20 and above, & 
C++ compiler
As for the depencencies, you can find them in the 'submodules' folder

## Dependencies
- [SFML 2.5.x](https://github.com/SFML/SFML/tree/2.5.x)
- [libcurl](https://github.com/curl/curl)
- [fmt](https://github.com/fmtlib/fmt)
- [args](https://github.com/Taywee/args)
- [inipp](https://github.com/mcmtroffaes/inipp)

## How to build
You need to clone the repo, after that you have to configure the file using cmake and compile it with the cmake configuration (you can see example commands in build.sh)
+ if you use windows and mingw you can just run the build shell script

## How to use
First, you will have to provide an offsets link to the program. To get the link go to https://www.minorplanetcenter.net/iau/NEO/toconfirm_tabular.html, manage the settings on the bottom of the site, select the objects, and click on "get ephemerides". After this, find the object that you want to observe, and click on one of its "Offsets" links, at the time you want to observe it on.

The program will download all the data needed, and display an uncertainty map. The controls are:
- Left Click to add an observation target
- Right Click to remove an observation target
- Q to exit the window and confirm the selection
- R to reset the view
- U to undo an observation target
- C to remove all observation targets
- You can zoom in and out with the scroll wheel
- Arrow keys to pan the view

After you have selected all observation areas, just close the uncertainty map window, and on the console you will see the coordinates of your targets. Also, the targets will automatically be copied to your clipboard.

## Variables
There are 3 variables that are user changeable
- window width (W)
- window height (H)
- telescope field of view (FOV)

All variables can be changed in the MPCS.ini file

> Telescope FOV is expressed in arcseconds

