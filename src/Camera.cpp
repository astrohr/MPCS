//----------------------------------------------------------

#include "Camera.hpp"

//----------------------------------------------------------

void Camera::reset_position(int telescope_FOV, ObjectDatabase& database){
    std::tie(m_offsetRa, m_offsetDec) = database.mean_center();
    auto [edgeRa, edgeDec] = database.mean_edges();
    edgeRa *= 2; edgeDec *= 2;
    edgeRa = std::max(telescope_FOV*1.3f, edgeRa);
    edgeDec = std::max(telescope_FOV*1.3f, edgeDec);
    m_zoomFactor = std::min((float)m_width/edgeRa, (float)m_height/edgeDec) * 0.9f;
}

void Camera::pan_camera(int deltaX, int deltaY){
    m_offsetRa += (float)deltaX * m_width/m_zoomFactor * 0.005f;
    m_offsetDec += (float)deltaY * m_height/m_zoomFactor * 0.005f;
}

const std::tuple<float, float> Camera::px_to_off(int x, int y) {
    float x_to_center = (float)m_width/2.f - x, y_to_center = (float)m_height/2.f - y;
    float x_off = x_to_center/m_zoomFactor + m_offsetRa;
    float y_off = y_to_center/m_zoomFactor + m_offsetDec;
    return {x_off, y_off};
}

void Camera::change_zoom(float delta, float x, float y){
    m_offsetRa += ((m_width/2.f - x) * 0.05f * delta) / m_zoomFactor;
    m_offsetDec += ((m_height/2.f - y) * 0.05f * delta) / m_zoomFactor;
    m_zoomFactor += m_zoomFactor * 0.05f * delta;
}
