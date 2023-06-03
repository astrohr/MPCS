// Camera.cpp ----------------------------------------------
// this file defines all things from Camera.hpp

#include "Camera.hpp"

//----------------------------------------------------------



const float Camera::raOffset() const { return m_offsetRa; }
const float Camera::decOffset() const { return m_offsetDec; }

const int Camera::window_w() const { return W; }
const int Camera::window_h() const { return H; }

const float Camera::zoom() const { return m_zoomFactor; }
const float Camera::view_w() const { return W/m_zoomFactor; }
const float Camera::view_h() const { return H/m_zoomFactor; }

void Camera::reset_position(int telescope_FOV, ObjectDatabase* database){
    std::tie(m_offsetRa, m_offsetDec) = database->mean_center();
    float edgeRa, edgeDec;
    std::tie(edgeRa, edgeDec) = database->mean_edges();
    edgeRa *= 2; edgeDec *= 2;
    edgeRa = std::max(telescope_FOV*1.3f, edgeRa);
    edgeDec = std::max(telescope_FOV*1.3f, edgeDec);
    m_zoomFactor = std::min((float)W/edgeRa, (float)H/edgeDec) * 0.9f;
}

void Camera::pan_camera(int deltaX, int deltaY){
    m_offsetRa += (float)deltaX * W/m_zoomFactor * 0.005f;
    m_offsetDec += (float)deltaY * H/m_zoomFactor * 0.005f;
}

const std::tuple<float, float> Camera::px_to_off(int x, int y) {
    float x_to_center = (float)W/2.f - x, y_to_center = (float)H/2.f - y;
    float x_off = x_to_center/m_zoomFactor + m_offsetRa;
    float y_off = y_to_center/m_zoomFactor + m_offsetDec;
    return {x_off, y_off};
}

void Camera::change_zoom(float delta, float x, float y){
    m_offsetRa += ((W/2.f - x) * 0.05f * delta) / m_zoomFactor;
    m_offsetDec += ((H/2.f - y) * 0.05f * delta) / m_zoomFactor;
    m_zoomFactor += m_zoomFactor * 0.05f * delta;
}

void Camera::change_dimensions(int x, int y){ W = x; H = y; }
void Camera::change_dimensions(unsigned int x, unsigned int y){ W = (int)x; H = (int)y; }

Camera::Camera(int x/*=1080*/, int y/*=920*/) : W(x), H(y) {
    m_offsetRa = 0.f;
    m_offsetDec = 0.f;
}
