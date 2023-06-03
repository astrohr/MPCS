#pragma once
// Camera.hpp ----------------------------------------------
// this file controls the camera in the UI

#include "pch.hpp"
// Precompiled headers in this file:
// cmath
// tuple

#include "ObjectDatabase.hpp"

//----------------------------------------------------------



class Camera{
private:
    //camera location in offset coords
    float m_offsetRa, m_offsetDec;
    //camera location in absolute coords [NOT IMPLEMENTED YET]
    float m_ra_abs, m_dec_abs;
    //zoom factor -> ratio of window size in px and arcseconds that fit in the window
    float m_zoomFactor;
    //window dimensions in px
    int W, H;
public:
    const float raOffset() const;
    const float decOffset() const;

    const int window_w() const;
    const int window_h() const;

    const float zoom() const;
    const float view_w() const;
    const float view_h() const;

    void reset_position(int telescope_FOV, ObjectDatabase* database);

    void pan_camera(int deltaX, int deltaY);

    const std::tuple<float, float> px_to_off(int x, int y);

    void change_zoom(float delta, float x, float y);

    void change_dimensions(int x, int y);
    void change_dimensions(unsigned int x, unsigned int y);

    Camera(int x = 1080, int y = 920);

};