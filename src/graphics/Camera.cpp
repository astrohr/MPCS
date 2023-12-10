//----------------------------------------------------------

#include "Camera.hpp"

//----------------------------------------------------------


Camera::Camera(float aspectRatio, float fov)
: position({0.0, 0.0, 0.0}), aspectRatio(aspectRatio), fov(fov), orientation(glm::quat(1.f, 0.f, 0.f, 0.f)), rotation(glm::quat(1.f, 0.f, 0.f, 0.f))
{}

void Camera::setAspectRatio(float ar)
{
    aspectRatio = ar;
}

void Camera::refresh()
{
    // recalculate where the camera is looking at
    // converts the rotation into the system of the orientation, rotates the system, and returns it back to normal
    lookingAt = glm::conjugate(orientation) * rotation * orientation;

    // recalculate the transformation matrix
    matrix = 
        glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 5.0f) 
        * glm::toMat4(lookingAt) 
        * glm::translate(glm::mat4(1.0f), glm::vec3(position.X, position.Y, position.Z))
    ;
}

void Camera::setOrientation(CoordinatesGeo& coords, time_t time)
{
    float roll, pitch, yaw; // roll = X, pitch = Y, yaw = Z

    // if cameras rotations are all at 0, the vernal equinox is in the zenith

    // account for the longitude 

    roll = (coords.lon / 360.f);

    // calculate the hour angle

    time_t HA = (time - g_siderealTimeReference.first + g_siderealTimeReference.second) % g_siderealDayLength; // hour angle
    if (HA < 0) HA += g_siderealDayLength; // if it is smaller than 0 make sure its within a valid range

    // abs(lat) is the highest point of the sky equator (Hour Angle = 0)
    // -abs(lat) is the lowest point of the sky equator (Hour Angle = 12)

    // rotate the camera along the sky equator towards the vernal equinox using the sidereal time reference

    yaw = (HA / g_siderealDayLength) * (2 * (float)std::numbers::pi);
    if (HA > g_siderealDayLength / 2)
        pitch = (3 - HA / (g_siderealDayLength / 4)) * glm::radians(coords.lat);
    else
        pitch = (1 - HA / (g_siderealDayLength / 4)) * glm::radians(-1 * coords.lat);

    orientation = glm::quat(glm::vec3(pitch, yaw, roll));
}

void Camera::setRotation(CoordinatesSkyLocal pos)
{
    rotation = glm::quat(glm::vec3(glm::radians(pos.alt), glm::radians(pos.az), 0.f));
}

void Camera::updateRotation(CoordinatesSkyLocal delta)
{
    rotation = glm::quat(glm::vec3(glm::radians(delta.alt), 0.f, 0.f)) * rotation * glm::angleAxis(glm::radians(1.0f), glm::vec3(0.f, delta.az, 0.f));
}

void Camera::zoom(bool closer)
{
    if (closer) fov = std::max(5.f, fov*0.985f);
    else fov = std::min(170.f, fov*1.015f);
}