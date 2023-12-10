//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

class Camera {
private:

    // the camera position
    Coordinates3D position;

    // a quaternion describing the orientation of the camera
    glm::quat orientation;
    // a quaternion describing the rotation of the camera
    glm::quat rotation;
    // a quaternion describing where the camera is looking at (combination of rotation and orientation)
    glm::quat lookingAt;

    // camera aspect ratio
    float aspectRatio;

    // camera fov (in degrees)
    float fov;

    // the transformation matrix of the camera (rotation, translation and projection)
    glm::mat4 matrix;

public:

    Camera(float aspectRatio, float fov);

    ~Camera() = default;


    // this function recalculates the total rotation of the camera and the transformation matrix
    void refresh();


    // retrieve the camera transformation
    const glm::mat4& getTransformation() const { return matrix; }

    // retrieve camera fov
    const float getFov() const { return fov; }

    // retrieve camera orientation
    const glm::quat& getOrientation() const { return orientation; }

    // retrieve camera rotation
    const glm::quat& getRotation() const { return rotation; }


    // change aspect ratio
    void setAspectRatio(float ar);

    // calculates how the camera should be rotated thanks to the provided time and location and look direction
    void setOrientation(CoordinatesGeo& coords, time_t time);

    // looks at a specific spot in the sky
    void setRotation(CoordinatesSkyLocal pos);


    // updates the camera position when informed many seconds had passed
    void updateOrientation(time_t deltatime) = delete;

    // updates the camera position when informed where it should look next
    void updateRotation(CoordinatesSkyLocal delta);

    // zooms the camera
    // \param[in] closer true if zoom in, false iz zoom out
    void zoom(bool closer);

};
