//----------------------------------------------------------

#include "Camera.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//----------------------------------------------------------


Camera::Camera(float window_W, float window_H, float fov)
: position(glm::vec3(0.f, 0.f, 0.f)), window_W(window_W), window_H(window_H), fov(fov), orientation(glm::quat(1.f, 0.f, 0.f, 0.f)), rotation({0.f, 0.f}), location({0.f, 0.f})
{}

void Camera::refresh()
{
    // recalculate where the camera is looking at

    // turn the rotation into a quaternion
    glm::quat rotationQuat = 
        glm::quat(glm::radians(rotation.alt) * glm::vec3(-1.f, 0.f, 0.f))
        * glm::quat(1.f, 0.f, 0.f, 0.f) 
        * glm::angleAxis(glm::radians(rotation.az), glm::vec3(0.f, 1.f, 0.f))
    ;

    // recalculate the transformation matrix
    matrix = 
        glm::perspective(glm::radians(fov), window_W/window_H, 0.1f, 5.0f) 
        * glm::toMat4(rotationQuat) * glm::toMat4(orientation) 
        * glm::translate(glm::mat4(1.0f), position)
    ;
}

void Camera::setOrientation(CoordinatesGeo& coords, time_t time)
{
    // by default the vernal equinox (ra, dec = 0, 0) will be 90° to the "left" of the X axis, and 90° to the right there will be the Z axis, Y axis will be in the zenith
    // why is this? i am not sure, but it would be cool if, by default, the vernal equinox would be in the in the direction of the X axis, and zenith in Z
    // that could be interpreted as standing on the north pole
    // since using Alt/Az on the north pole can be confusing, we would the move to the equator to the coordinates 0,0 where in this configuration
    // the vernal equinox will be in the zenith, and we shall also turn the camera towards the north (Z axis)
    // this can be interpreted as standing on the location 0,0 on the earth with the VE in the zenith and would be easy to work with
    glm::quat correctionRotation = 
        glm::quat(glm::radians(90.f) * glm::vec3(1.f, 0.f, 0.f)) // move the X axis to the zenith
        * glm::quat(glm::radians(90.f) * glm::vec3(0.f, 0.f, -1.f)) // move the Z axis to the nadir and Y axis to the "right" of the X
        * glm::quat(glm::radians(90.f) * glm::vec3(0.f, 1.f, 0.f)) // move X axis in front
    ;

    // now, a rotation describing where the vernal equinox is at those coordinates has to be created because it certainly wont be in the zenith
    // and a rotation describing where on the planet the observer is located has to be done
    // those rotations will describe the orientation of the camera, and the view from given coordinates at given time

    // the position of the vernal equinox is calculated with two angles
    // first is the hour angle of the vernal equinox
    // second is a "roll" that describes the angle between the equator and the ecliptic because as time progresses, this angle changes

    // the hour angle of the vernal equinox is also known as the "local star time" or in this case "greenwich star time", a synonim for a sidereal day
    
    // we have to set the location to 0, 0 because currently we are operating from there
    location = {0.f, 0.f};

    // calculate the location of VE time is assumed to be in UTC0 (the function will recieve unix time so thats working ok)
    CoordinatesSkyLocal veLoc = SkyToSkyLocal({0.f, 0.f}, time);
    
    // now using the coordinates, we have to move the camera in such a way that, to us, it looks like the vernal equinox is in the coordinates that we got
    // so if alt is supposed to be 10°, we have to move the camera 80° down because currently VE is at an alt of 90°
    // the order of transformations is "alt first, az second" because if we rotate a point that is at alt 90° around the azimuth coordinates we will change nothing
    glm::quat veRotation = 
        (glm::quat(1.f, 0.f, 0.f, 0.f) * glm::angleAxis(glm::radians(veLoc.az), glm::vec3(0.f, -1.f, 0.f)))
        * glm::quat(glm::radians(90.f - veLoc.alt) * glm::vec3(1.f, 0.f, 0.f)) 
    ;

    // veRotation will point the camera in the direction of the vernal equinox, but we will still be looking at it from the percpective of the 0, 0
    // we must rotate the camera accordingly to the position on the planet
    glm::quat geoRotation = 
        glm::quat(glm::radians(coords.lat) * glm::vec3(-1.f, 0.f, 0.f)) 
        * glm::quat(1.f, 0.f, 0.f, 0.f)
        * glm::angleAxis(glm::radians(coords.lon), glm::vec3(0.f, 0.f, 1.f))
    ;

    // and now we combine the 3 rotations
    //orientation = geoRotation * veRotation * correctionRotation;

    // set the camera geographic location for further calculations
    location = coords;
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
    else fov = std::min(120.f, fov*1.015f);
}

CoordinatesSkyLocal Camera::SkyToSkyLocal(CoordinatesSky coords, time_t time)
{
    // calculate the local hour angle
    float hourAngle = ((float)getGMST(time) / g_siderealDayLength * 2.f * (float)std::numbers::pi) + glm::radians(location.lon) - glm::radians(coords.ra);
    if (hourAngle < 0) hourAngle += std::numbers::pi * 2;
    if (hourAngle > 2.f * std::numbers::pi) hourAngle -= std::numbers::pi * 2;

    return{
        glm::degrees(std::atan2(
            std::sin(hourAngle), 
             std::cos(hourAngle) * std::sin(glm::radians(location.lat)) 
             - std::tan(glm::radians(coords.dec)) * std::cos(glm::radians(location.lat))
        )), // azimuth
        glm::degrees(std::asin(
            std::sin(glm::radians(location.lat)) * std::sin(glm::radians(coords.dec)) 
            + std::cos(glm::radians(location.lat)) * std::cos(glm::radians(coords.dec)) * std::cos(hourAngle)
        )) // altitude
    };
    //https://astrogreg.com/convert_ra_dec_to_alt_az.html
}

CoordinatesSkyLocal Camera::screenToSkyLocal(float X, float Y)
{
    // Convert screen coordinates to normalized device coordinates (NDC)
    glm::vec3 ndcCoords(
        (2.0f * X) / window_W - 1.0f, 
        1.0f - (2.0f * Y) / window_H, 
        0.0f
    );
    glReadPixels(X, window_H - Y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &ndcCoords[2]);

    glm::quat rotationQuat = 
        glm::quat(glm::radians(rotation.alt) * glm::vec3(-1.f, 0.f, 0.f))
        * glm::quat(1.f, 0.f, 0.f, 0.f) 
        * glm::angleAxis(glm::radians(rotation.az), glm::vec3(0.f, 1.f, 0.f))
    ;

    // this function will turn the position on screen into a vector where
    // x = azimuth coordinates but in range [-0.1, 0.1] for some reason
    // y = altitude coordinates in range [-0.1, 0.1]
    glm::vec3 unproj = glm::unProject(
        ndcCoords,                                              // position
        glm::toMat4(rotationQuat) * glm::toMat4(this->orientation),         // view matrix
        glm::perspective(glm::radians(fov), window_W/window_H, 0.1f, 5.0f), // perspective matrix
        glm::vec4(0.f, 0.f, this->window_W, this->window_H)                 // viewport
    );

    fmt::println("{} {} {}", unproj[0] * 10.f, unproj[1] * 10.f, unproj[2]);

    return {0.f, 0.f};

    /*
    // X and Y begin in the lower left corner, so can turn this into a cartesian coordinate system like this:
    float x = X - window_W/2.f, y = window_H/2.f - Y;

    // calculate the vector that describes where the camera is looking at
    // first get the rotation quaternion
    glm::quat rotationQuat = 
        glm::quat(glm::radians(rotation.alt) * glm::vec3(-1.f, 0.f, 0.f))
        * glm::quat(1.f, 0.f, 0.f, 0.f) 
        * glm::angleAxis(glm::radians(rotation.az), glm::vec3(0.f, 1.f, 0.f))
    ;
    // then apply it on the position where camera looks at with no rotations present
    glm::vec4 cameraVec = glm::vec4(0.f, 0.f, -1.f, 0.f) * glm::mat4(rotationQuat * orientation);

    // fmt::println("First =  {}  {}  {}  {}", cameraVec[0], cameraVec[1], cameraVec[2], cameraVec[3]);

    // now we need to figure out where the cursor is pointing
    // calculate the "angular distance" of the cursor from the middle of the screen

    // we know that the FOV angle goes from the top of the camera field of view to the bottom, so along the Y axis
    // using that, we can calculate the "distance in pixels" of the screen from the camera, lets call that value k
    // (window_H/2) / k = tan(fov/2) is the main idea, we will calculate the constant k, and then when we get some cursor height value y
    float k = (window_H/2.f) / std::tan(glm::radians(fov/2.f));

    // X and Y begin in the lower left corner, so can turn this into a cartesian coordinate system like this:
    float x = X - window_W/2.f, y = window_H/2.f - Y;

    // now we should be able to calculate the angle using the formula y/k = tan(angle)
    // and while that would work great in a 2D scenario, we are working in 3D so we need to think about the projection on the X axis too
    // in other words if something is height y, its altitude can change depending on the x, because of the projection
    // the solution to this is to imagine many concentric circles from the middle (0,0), on each of those, the angle from the middle is the same

    // we calculate the middle from the distance in pixels
    float midDist = std::sqrt(std::pow(x, 2) + std::pow(y, 2));

    // now we can calculate the angular distance from the middle (0,0), and use it
    float angDist = std::atan2(midDist, k);

    // we can now split this distance over the x y components
    float angX = angDist * std::sin(std::atan2(y,x));
    float angY = angDist * std::cos(std::atan2(y,x));

    // fmt::println("angs =  {}  {}", glm::degrees(angX), glm::degrees(angY));

    // now that we split it, we can add those values to cameraVec to get a vector of where our cursor is pointing
    glm::quat localRotationQuat = 
        glm::quat(angX * glm::vec3(-1.f, 0.f, 0.f))
        * glm::quat(1.f, 0.f, 0.f, 0.f) 
        * glm::angleAxis(angY, glm::vec3(0.f, 1.f, 0.f))
    ;
    cameraVec = cameraVec * glm::mat4(localRotationQuat);

    // fmt::println("Second =  {}  {}  {}  {}", cameraVec[0], cameraVec[1], cameraVec[2], cameraVec[3]);

    // normalize the vector and convert its coordinates into polar coordinates
    glm::vec3 polar = glm::polar(glm::vec3(glm::normalize(cameraVec)));

    // according to the documentation x is the xz distance, y, the latitude and z the longitude
    return {glm::degrees(polar[2]), glm::degrees(polar[1])};
    */
}

CoordinatesSky Camera::screenToSky(float X, float Y, time_t time)
{
    // get local coordinates
    CoordinatesSkyLocal local = screenToSkyLocal(X, Y);

    // hour angle
    float H = std::atan2(
        - std::sin(glm::radians(local.az)), 
        std::tan(glm::radians(local.alt))*std::cos(glm::radians(location.lat)) - std::cos(glm::radians(local.az))*std::sin(glm::radians(location.lat))
    ); 
    if (H < 0) H += 2.f * std::numbers::pi;
    float ra = ((float)getGMST(time) / g_siderealDayLength * 2.f * (float)std::numbers::pi) + glm::radians(location.lon) - H;
    if (ra < 0) ra += 2.f * std::numbers::pi;
    if (ra > 2.f * std::numbers::pi) ra -= 2.f * std::numbers::pi;
    return{
        glm::degrees(ra),
        glm::degrees(std::asin(
            std::sin(glm::radians(location.lat)) * std::sin(glm::radians(local.alt)) 
            + std::cos(glm::radians(location.lat)) * std::cos(glm::radians(local.alt))*std::cos(glm::radians(local.az))
        ))
    };

    // https://astrogreg.com/snippets/altaztoHAdec.html
    // https://en.wikipedia.org/wiki/Hour_angle
}

CoordinatesSky Camera::screenToSky_HA(float X, float Y, time_t time)
{
    // get local coordinates
    CoordinatesSkyLocal local = screenToSkyLocal(X, Y);

    // hour angle
    float H = std::atan2(
        - std::sin(glm::radians(local.az)),
        std::tan(glm::radians(local.alt))*std::cos(glm::radians(location.lat)) - std::cos(glm::radians(local.az))*std::sin(glm::radians(location.lat))
    ); 
    if (H < 0) H += 2.f * std::numbers::pi;
    return{
        glm::degrees(H),
        glm::degrees(std::asin(
            std::sin(glm::radians(location.lat)) * std::sin(glm::radians(local.alt)) 
            + std::cos(glm::radians(location.lat)) * std::cos(glm::radians(local.alt)) * std::cos(glm::radians(local.az))
        ))
    };

    // https://astrogreg.com/snippets/altaztoHAdec.html
    // https://en.wikipedia.org/wiki/Hour_angle
}
