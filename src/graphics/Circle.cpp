//----------------------------------------------------------

#include "Graphics.hpp" 
#include "Circle.hpp"
#include "PointCluster.hpp"

//----------------------------------------------------------

Circle::Circle(const glm::vec4& position, const glm::quat& rotation, const glm::vec4& rgba, const float radius, const int resolution/*=64*/)
: position(position), rotation(rotation), rgba(rgba), radius(radius), resolution(resolution)
{
    createCircleVertices(vertices, position, rotation, radius, resolution);
    std::tie(VA, VB) = PointCluster::createPointsBuffer(vertices, rgba);
}
Circle::Circle(const glm::vec4& position, const glm::vec4& normal, const glm::vec4& rgba, const float radius, const int resolution/*=64*/)
: position(position), rgba(rgba), radius(radius), resolution(resolution)
{
    glm::quat r = glm::rotation(glm::vec3(position), glm::vec3(0.f, 1.f, 0.f));
    Circle(position, r, rgba, radius, resolution);
}

Circle::~Circle()
{
    glDeleteBuffers(1, &VB);
    glDeleteVertexArrays(1, &VA);
}

void Circle::draw() const
{
    glBindVertexArray(VA);
    glDrawArrays(GL_LINE_LOOP, 0, resolution);
}

void Circle::createCircleVertices(
    std::vector<glm::vec4>& returnData, const glm::vec4 &position, const glm::quat &rotation, const float radius, const int resolution/*=64*/)
{
    for(int i = 0; i < resolution; i++){
        returnData.emplace_back( 
            // create the point of the unit circle
            glm::vec4(
                std::cos(fPi * 2.f * (float)i / (float)resolution), 0.f,
                std::sin(fPi * 2.f * (float)i / (float)resolution), 0.f
            )
            // rotate, scale and translate
            * glm::mat4(rotation) * radius + position
        );
    }
}
