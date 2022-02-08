# MPCS
A program for solving the neo uncertainty

MPCS stands for "Minor Planet Center Solver"

## Requirements
A Windows machine
Curl https://curl.se/ (comes preinstalled with most Windows 10 machines)
> you can check if you have curl by typing "curl --version" into your cmd, without quotations of course

## How to use
Download the latest release, unzip it, and run the exe file

First, you will have to provide an offsets link to the program. To get the link go to https://www.minorplanetcenter.net/iau/NEO/toconfirm_tabular.html, manage the settings on the bottom of the site, select the objects, and click on "get ephemerides". After this, find the object you want to observe, and click on one of its "Offsets" links, at the time you want to observe it.
The program will download all the data needed, and display an uncertainty map. The controls are:
- Left Click to add an observation target
- Right Click to remove an observation target
- Q to exit the window and confirm the selection
- R to reset the view
- U to undo an observation target
- C to remove all observation targets
- You can zoom in and out with the scroll wheel

After you have selected all observation areas, just close the uncertainty map window, and on the console you will see the coordinates of your targets. Also, the targets will automatically be copied to your clipboard.

## Variables
There are 3 variables that are user changeable
- window height
- window width
- telescope field of view
All variables can be changed in the variables.txt file
> Telescope FOV is expressed in arcseconds
> It is suggested that you have width and height using equal values
