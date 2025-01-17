//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

class Circle{
private:
    glm::vec4 position; // the 3d coordinates of the circle
    glm::quat rotation; // the normal of the circle (rotation)
    float radius;

    int resolution; // how many vertices for this circle

    std::vector<glm::vec4> vertices; // circle vertices

    glm::vec4 rgba; // rgba color

    unsigned int VA, VB;

public:
    Circle(const glm::vec4& position, const glm::quat& rotation, const glm::vec4& rgba, const float radius, const int resolution = 64);
    Circle(const glm::vec4& position, const glm::vec4& normal, const glm::vec4& rgba, const float radius, const int resolution = 64);
    ~Circle();

    void draw() const;

    static void createCircleVertices(
        std::vector<glm::vec4>& returnData, const glm::vec4& position, const glm::quat& rotation, const float radius, const int resolution = 64
    );
};