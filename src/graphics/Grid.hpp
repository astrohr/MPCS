//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

class Grid{
private:
    glm::vec4 location; // the location of the grid, the "middle" of the circles
    glm::vec4 center; // where is the center of the grid "where is the camera looking at"
    glm::quat rotation; // the rotation of the grid

    float circle_radius;
    int circle_resolution;
    int W_amount, H_amount; // amount of circles in width and height

    unsigned int VA, VB;

    void calculateVertices(int W_amount, int H_amonut, std::vector<glm::vec4>& returnData);
public:
    Grid(const glm::vec4& location, const glm::vec4& center, const glm::quat& rotation, int W_amount, int H_amount, float circle_radius = 1.f);
    ~Grid();

    void updateResolution(int W_amount, int H_amonut);

    void draw() const;
};