//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

class Camera {
private:

    // the camera position
    Coordinates3D position;

    // the camera orientation (rotation) angles (in radians) [roll(X), pitch(Y), yaw(Z)]
    glm::vec3 rotations;

    // the size of camera screen
    int W, H;

    // camera fov (in degrees)
    float fov;

    // what is the camera looking at?
    // default looking position {0,0} is north
    CoordinatesSkyLocal lookingPosition;

    // the transformation matrix of the camera (rotation, translation and projection)
    glm::mat4 matrix;
    void calibrateCameraMatrix();


public:

    Camera(int W, int H, float fov);

    ~Camera() = default;


    // retrieve the camera transformation
    const glm::mat4 getTransformation() const { return matrix; }
    // retrieve the camera rotations [roll(X), pitch(Y), yaw(Z)]
    const glm::vec3& getRotationAngles() const { return rotations; }
    // retrieve the looking position in the sky
    const CoordinatesSkyLocal getLookingPosition() const { return lookingPosition; }

    // calculates how the camera should be rotated thanks to the provided time and location and look direction
    void setRotations(CoordinatesGeo& coords, time_t time, CoordinatesSkyLocal look = {0,0});

    // updates the camera position when informed many seconds had passed
    void updateRotations(time_t deltatime) = delete;
    // updates the camera position when informed where it should look next
    void updateRotations(CoordinatesSkyLocal newPosition);

    // pans the camera
    void pan(bool up, bool left, bool down, bool right);
    
    // zooms the camera
    // \param[in] closer true if zoom in, false iz zoom out
    void zoom(bool closer);
};
