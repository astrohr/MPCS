//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

class Camera {
private:

    // the camera position
    glm::vec3 position;

    // a quaternion describing the orientation of the camera
    glm::quat orientation;
    // local rotation of the camera in the ALT/AZ system
    CoordinatesSkyLocal rotation;
    // where is the camera observing from?
    CoordinatesGeo location;

    // window dimensions
    float window_W, window_H;

    // camera fov (in degrees)
    float fov;

    // the transformation matrix of the camera (rotation, translation and projection)
    glm::mat4 matrix;

public:

    Camera(float window_W, float window_H, float fov);

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
    const CoordinatesSkyLocal& getRotation() const { return rotation; }

    // change aspect ratio
    void setWindowDimensions(float window_W, float window_H) { this->window_W = window_W; this->window_H = window_H; };

    // calculates how the camera should be rotated thanks to the provided time and location and look direction
    // IMPORTANT: the time provided to this function has to be UTC0
    void setOrientation(CoordinatesGeo& coords, time_t time);

    // looks at a specific spot in the sky
    void setRotation(CoordinatesSkyLocal pos) { rotation = pos; };


    // updates the camera position when informed many seconds had passed
    void updateOrientation(time_t deltatime) = delete;

    // updates the camera position when informed where it should look next
    void updateRotation(CoordinatesSkyLocal delta);

    // zooms the camera
    // \param[in] closer true if zoom in, false iz zoom out
    void zoom(bool closer);

    // convert from alt/az to ra/dec
    CoordinatesSkyLocal SkyToSkyLocal(CoordinatesSky coords, time_t time);
    // calculates where the position on the screen is in the Alt/Az coordinates
    CoordinatesSkyLocal screenToSkyLocal(float X, float Y);
    // calculates where the position on the screen is in the Dec/Ra coordinates
    CoordinatesSky screenToSky(float X, float Y) = delete; // unimplemented? not for long

};
