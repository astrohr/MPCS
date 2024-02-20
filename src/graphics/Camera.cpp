//----------------------------------------------------------

#include "Camera.hpp"

//----------------------------------------------------------


Camera::Camera(float window_W, float window_H, float fov)
: position(glm::vec3(0.f, 0.f, 0.f)), window_W(window_W), window_H(window_H), fov(fov), orientation(glm::quat(1.f, 0.f, 0.f, 0.f)), rotation({0.f, 0.f}), location({0.f, 0.f})
{}

void Camera::refresh()
{
    // recalculate where the camera is looking at

    // turn the rotation into a quaternion
    glm::quat rotationQuat = 
        glm::quat(glm::radians(rotation.alt) * glm::vec3(1.f, 0.f, 0.f))
        * glm::quat(1.f, 0.f, 0.f, 0.f) 
        * glm::angleAxis(glm::radians(rotation.az), glm::vec3(0.f, 1.f, 0.f))
    ;

    // recalculate the transformation matrix
    matrix = 
        glm::perspective(glm::radians(fov), window_W/window_H, 0.1f, 5.0f) 
        * glm::toMat4(rotationQuat * orientation) 
        * glm::translate(glm::mat4(1.0f), position)
    ;
}

void Camera::setOrientation(CoordinatesGeo& coords, time_t time)
{
    // by default (orientation = 0 and lat, lon = 0, 0) the vernal equinox (ra, dec = 0, 0) will be right in the zenith
    // firstly a rotation describing where the vernal equinox is at those coordinates has to be created because it certainly wont be in the zenith
    // secondly a rotation describing where on the planet the observer is located has to be done
    // those rotations combined describe the orientation of the camera, and the view from given coordinates at given time
    // it is also an important information that without any additional rotations, the camera will be facing the zenith

    // the position of the vernal equinox is calculated with two angles
    // first is the hour angle of the vernal equinox
    // second is a "roll" that describes the angle between the equator and the ecliptic because as the year progresses, this angle changes

    // the hour angle of the vernal equinox is also known as the "local star time" or in this case "greenwich star time", a synonim for a sidereal day

    location = coords; // set the camera geographic location
    
    // calculate the location of VE (time is assumed to be in UTC0)
    CoordinatesSkyLocal veLoc = SkyToSkyLocal({0.f, 0.f}, time);
    
    glm::quat veRotation = 
        glm::quat(glm::radians(veLoc.alt) * glm::vec3(1.f, 0.f, 0.f)) 
        * glm::quat(1.f, 0.f, 0.f, 0.f) 
        * glm::angleAxis(glm::radians(veLoc.az), glm::vec3(0.f, 1.f, 0.f))
    ;

    orientation = glm::quat(glm::radians(-90.f) * glm::vec3(1.f, 0.f, 0.f)) * veRotation; // additional transformation so that camera doesnt look into the zenith
}

void Camera::updateRotation(CoordinatesSkyLocal delta)
{
    rotation.alt = std::max(-90.f, std::min(90.f, rotation.alt + delta.alt));
    rotation.az += delta.az;
    rotation.az = std::fmod(rotation.az, 360.f);
    if (rotation.az < 0) rotation.az += 360.f;
}

void Camera::zoom(bool closer)
{
    if (closer) fov = std::max(5.f, fov*0.985f);
    else fov = std::min(170.f, fov*1.015f);
}

CoordinatesSkyLocal Camera::SkyToSkyLocal(CoordinatesSky coords, time_t time)
{
    // calculate the local hour angle
    float hourAngle = std::fmod(((float)getGMST(time) / g_siderealDayLength * 2 * std::numbers::pi) + glm::radians(location.lon) - glm::radians(coords.ra), std::numbers::pi * 2);
    if (hourAngle < 0) hourAngle += std::numbers::pi * 2;

    return{
        glm::degrees(std::atan2(std::sin(hourAngle), std::cos(hourAngle) * std::sin(glm::radians(location.lat)) - std::tan(glm::radians(coords.dec)) * std::cos(glm::radians(location.lat)))), // azimuth
        glm::degrees(std::asin(std::sin(glm::radians(location.lat)) * std::sin(glm::radians(coords.dec)) + std::cos(glm::radians(location.lat)) * std::cos(glm::radians(coords.dec)) * std::cos(hourAngle))) // altitude
    };
    // more explanations can be found on
    // https://astrogreg.com/convert_ra_dec_to_alt_az.html
}

CoordinatesSkyLocal Camera::screenToSkyLocal(float X, float Y)
{
    CoordinatesSkyLocal local;

    // calculate local location
    local.alt = (Y - window_H/2.f) / window_H * fov;
    local.az = (X - window_W/2.f) / window_W * fov * (window_W / window_H);

    // add camera values
    local.alt += rotation.alt;
    local.az += rotation.az;

    // normalize
    local.alt = 90.f * std::sin(glm::radians(local.alt));
    local.az = std::fmod(local.az, 360.f);
    if (local.az < 0) local.az += 360.f;

    return local;
}

// CoordinatesSky Camera::screenToSky(float X, float Y)
// {
//     // get local coordinates
//     CoordinatesSkyLocal local = screenToSkyLocal(X, Y);

//     // convert them to sky coordinates

// }
