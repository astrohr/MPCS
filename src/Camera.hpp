//----------------------------------------------------------

#pragma once

#include "pch.hpp"

#include "ObjectDatabase.hpp"

//----------------------------------------------------------



class Camera{
private:
    
    //camera location in offset coords
    float m_offsetRa, m_offsetDec;
    
    //zoom factor -> ratio of window size in px and arcseconds that fit in the window
    float m_zoomFactor;
    
    //window dimensions in px
    unsigned int m_width, m_height;

public:

    // Camera object constructor
    // \param W window width
    // \param H window height
    Camera(int W = 1080, int H = 920)
    : m_width(W), m_height(H), m_offsetRa(0.f), m_offsetDec(0.f) {}



    // Right ascension offset getter
    // \returns right ascension offset coordinate of the camera position
    const float raOffset() const
    { return m_offsetRa; }
    
    // Declination offset getter
    // \returns declination offset coordinate of the camera position
    const float decOffset() const
    { return m_offsetDec; }

    // Camera window width getter
    // \returns winow width that camera object considers relevant
    const unsigned int getWindow_W() const
    { return m_width; }
    
    // Camera window height getter
    // \returns winow height that camera object considers relevant
    const unsigned int getWindow_H() const
    { return m_height; }

    // Camera zoom getter
    // \returns the value of the camera zoom
    const float getZoom() const
    { return m_zoomFactor; }
    
    // Apparent view width getter
    // \returns how "the size of the width"
    const float getView_W() const
    { return m_width/m_zoomFactor; }
    
    // Apparent view height getter
    // \returns how "the size of the height"
    const float getView_H() const
    { return m_height/m_zoomFactor; }

    // Setter for window dimensions
    // \param W width
    // \param H height
    void setDimensions(unsigned int W, unsigned int H)
    { m_width = (int)W; m_height = (int)H; }



    // Reset the camera position to origin
    // \param telescope_FOV
    // \param ObjectDatabase
    void reset_position(int telescope_FOV, ObjectDatabase& database);

    // Pan the camera
    // \param deltaX amount if horizontal panning
    // \param deltaY amount if vertical panning
    void pan_camera(int deltaX, int deltaY);

    // Function that converts a pixel location on screen to an offset coordinate pair
    // \param x horizontal coordinate part
    // \param y vertical coordinate part
    const std::tuple<float, float> px_to_off(int x, int y);

    // Function for changing zoom
    // \param delta amount of zoom change
    // \param x cursor x position
    // \param y cursor y position
    void change_zoom(float delta, float x, float y);

};