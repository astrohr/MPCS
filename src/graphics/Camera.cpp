//----------------------------------------------------------

#include "Camera.hpp"

//----------------------------------------------------------


void Camera::calibrateCameraMatrix()
{
    glm::mat4 proj = glm::perspective(
        fov * (float)g_radian, // fov
        (float)W/H, // aspect ratio
        0.1f, // near clipping plane, keep as big as possible, or you get precision issues
        5.0f // far clipping plane, keep as little as possible
    );

    glm::vec3 EulerAngles(pitch, roll, yaw);
    glm::quat quaternion(EulerAngles);
    glm::mat4 rot = glm::toMat4(quaternion);
    
    glm::mat4 tran = glm::translate(glm::mat4(1.0f), glm::vec3(position.X, position.Y, position.Z));

    matrix = proj * rot * tran;

    // std::cout << "\nproj:\n";
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         std::cout << proj[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << "rot:\n";
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         std::cout << rot[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << "tran:\n";
    // for (int i = 0; i < 4; i++) {
    //     for (int j = 0; j < 4; j++) {
    //         std::cout << tran[i][j] << " ";
    //     }
    //     std::cout << std::endl;
    // }
}

Camera::Camera(Coordinates3D position, double pitch, double roll, double yaw, int W, int H, float fov)
: position(position), pitch(pitch), roll(roll), yaw(yaw), W(W), H(H), fov(fov), lookingPosition({0.0, 0.0})
{
    calibrateCameraMatrix();
}

Camera::Camera(int W, int H, float fov)
: position({0.0, 0.0, 0.0}), pitch(0.0), roll(0.0), yaw(0.0), W(W), H(H), fov(fov), lookingPosition({0.0, 0.0})
{
    calibrateCameraMatrix();
}

void Camera::setRotations(CoordinatesGeo& coords, time_t time, CoordinatesSkyLocal look)
{
    // if cameras rotations are all at 0, the vernal equinox is in the zenit

    // account for the longitude 

    roll = (coords.lon / 360.0);

    // calculate the hour angle

    time_t HA = (time - g_siderealTimeReference.first + g_siderealTimeReference.second) % g_siderealDayLength; // hour angle
    if (HA < 0) HA += g_siderealDayLength; // if it is smaller than 0 make sure its within a valid range

    // abs(lat) is the highest point of the sky equator (Hour Angle = 0)
    // -abs(lat) is the lowest point of the sky equator (Hour Angle = 12)

    // rotate the camera along the sky equator towards the vernal equinox using the sidereal time reference

    yaw = (HA / g_siderealDayLength) * (2 * std::numbers::pi);
    if (HA > g_siderealDayLength / 2) pitch = (3 - HA / (g_siderealDayLength / 4)) * (coords.lat * g_radian);
    else pitch = (1 - HA / (g_siderealDayLength / 4)) * (-coords.lat * g_radian);

    updateRotations(look); // this also updates the camera matrix

    // a small note that even tho it is not needed to use all 3 roll directions, it is done for the sake of simplicity
}

void Camera::updateRotations(CoordinatesSkyLocal &newPosition)
{
    yaw += (newPosition.alt - lookingPosition.alt) * g_radian;
    pitch += (newPosition.az - newPosition.az) * g_radian; 

    lookingPosition = newPosition;
    calibrateCameraMatrix();
}

void Camera::pan(bool up, bool left, bool down, bool right)
{
    if (up) pitch += 1*g_radian;
    if (left) yaw -= 1*g_radian;
    if (down) pitch -= 1*g_radian;
    if (right) yaw += 1*g_radian;
    calibrateCameraMatrix();
}