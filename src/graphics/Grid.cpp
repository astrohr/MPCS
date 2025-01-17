//----------------------------------------------------------

#include "Graphics.hpp"
#include "Grid.hpp"
#include "Circle.hpp"
#include "PointCluster.hpp"

//----------------------------------------------------------

void Grid::calculateVertices(int W_amount, int H_amonut, std::vector<glm::vec4>& returnData)
{
    for(int i = 0; i < W_amount; i++){
        // "meridian" spheres
        glm::quat sub_rotation = 
            glm::quat(1.f, 0.f, 0.f, 0.f) 
            * glm::angleAxis(glm::radians(180.f / (float)W_amount * (float)i), glm::vec3(1.f, 0.f, 0.f)) 
            * glm::angleAxis(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f))
            * rotation
        ;
        Circle::createCircleVertices(returnData, location, sub_rotation, circle_radius, circle_resolution);
    }
    for(int i = 0; i < H_amount; i++){
        // "parallel" spheres
        float h = circle_radius * (float)std::sin(glm::radians(-90.f + (i+1) * 180.f / (H_amount+1)));
        glm::vec4 sub_location = glm::vec4(0.f, h, 0.f, 0.f) * glm::mat4(rotation) + location;
        float sub_radius = std::sqrt(std::pow(circle_radius, 2) - std::pow(h,2));
        Circle::createCircleVertices(returnData, sub_location, rotation, sub_radius, circle_resolution);
    }
}

Grid::Grid(const glm::vec4& location, const glm::vec4& center, const glm::quat& rotation, int W_amount, int H_amount, float circle_radius/*=1*/)
: location(location), center(center), rotation(rotation), W_amount(W_amount), H_amount(H_amount), circle_radius(circle_radius), circle_resolution(64)
{
    std::vector<glm::vec4> points;
    calculateVertices(W_amount, H_amount, points);
    std::tie(VA, VB) = PointCluster::createPointsBuffer(points, glm::vec4(0.5f, 0.f, 0.f, 1.f));
}

Grid::~Grid()
{
    glDeleteBuffers(1, &VB);
    glDeleteVertexArrays(1, &VA);
}

void Grid::updateResolution(int W_amount, int H_amount)
{
    std::vector<glm::vec4> points;
    calculateVertices(W_amount, H_amount, points);

}

void Grid::draw() const
{
    glBindVertexArray(VA);
    GLint starts[W_amount+H_amount];
    GLsizei sizes[W_amount+H_amount];
    for(int i = 0; i < (W_amount+H_amount); i++){
        starts[i] = circle_resolution * i;
        sizes[i] = circle_resolution;
    }
    glMultiDrawArrays(GL_LINE_LOOP, starts, sizes, (W_amount+H_amount));
}
