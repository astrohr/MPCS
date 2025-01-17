//----------------------------------------------------------

#pragma once

#include "pch.hpp"
#include "utils/utils.hpp"

//----------------------------------------------------------

class PointCluster{
private:
    std::vector<glm::vec4> points;
    std::vector<glm::vec4> colors;

    unsigned int VA, VB;
public:
    PointCluster(const std::vector<glm::vec4>& points, const std::vector<glm::vec4>& colors);
    PointCluster(const std::vector<glm::vec4>& points, const glm::vec4& color);
    ~PointCluster();

    void draw();

    unsigned int getVB() const { return VB; }

    // creates multiple points (dots) in the buffer and returns its data
    // points is an array where each element is the location of the point
    // colors is an array where each element is the color of the point
    // sizes is an array where each element is the size of the point
    // returns tuple with vertex array id and buffer id (in that order)
    static const std::tuple<unsigned int, unsigned int> createPointsBuffer(
        const std::vector<glm::vec4>& points, const std::vector<glm::vec4>& colors
    );
    static const std::tuple<unsigned int, unsigned int> createPointsBuffer(
        const std::vector<glm::vec4>& points, const glm::vec4& color
    );
};