//----------------------------------------------------------

#include "PointCluster.hpp"
#include "Graphics.hpp"

//----------------------------------------------------------

PointCluster::PointCluster(const std::vector<glm::vec4>& points, const std::vector<glm::vec4>& colors)
: points(points), colors(colors)
{
    std::tie(VA, VB) = createPointsBuffer(points, colors);
}
PointCluster::PointCluster(const std::vector<glm::vec4>& points, const glm::vec4& color)
: points(points), colors(std::vector<glm::vec4>(points.size(), color))
{
    std::tie(VA, VB) = createPointsBuffer(points, color);
}


PointCluster::~PointCluster()
{
    glDeleteBuffers(1, &VB);
    glDeleteVertexArrays(1, &VA);
}

void PointCluster::draw()
{
    glBindVertexArray(VA);
    glDrawArrays(GL_POINTS, 0, points.size());
}

// creates multiple points (dots) in the buffer and returns its data
// points is an array where each element is the location of the point
// colors is an array where each element is the color of the point
// sizes is an array where each element is the size of the point
// returns tuple with vertex array id and buffer id (in that order)
const std::tuple<unsigned int, unsigned int> PointCluster::createPointsBuffer(
    const std::vector<glm::vec4>& points, const std::vector<glm::vec4>& colors
){
    // first we insert the coordinates into the buffer
    std::vector<float> vertices;
    for(auto point : points){
        vertices.emplace_back(point[0]); // X
        vertices.emplace_back(point[1]); // Y
        vertices.emplace_back(point[2]); // Z
    }
    // then we insert the RGB
    for(auto color : colors){
        vertices.emplace_back(color[0]); //R
        vertices.emplace_back(color[1]); //G
        vertices.emplace_back(color[2]); //B
    }
    // then we insert the alpha channel which is separated because it gets modified way more often
    for(auto color : colors){
        vertices.emplace_back(color[3]); //A
    }

    unsigned int VAID, BID;

    // create object IDs
    glGenVertexArrays(1, &VAID);
    glGenBuffers(1, &BID);

    // bind
    glBindVertexArray(VAID);
    glBindBuffer(GL_ARRAY_BUFFER, BID);

    // insert the data
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_DYNAMIC_DRAW);

    // specify the layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // coords layout
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(points.size()*3*sizeof(float))); // rgb channels layout
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)(points.size()*6*sizeof(float))); // alpha channel layout
    glEnableVertexAttribArray(2);

    // unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return {VAID, BID};
}
const std::tuple<unsigned int, unsigned int> PointCluster::createPointsBuffer(
    const std::vector<glm::vec4>& points, const glm::vec4& color
){
    std::vector<glm::vec4> colors(points.size(), color);
    return createPointsBuffer(points, colors);
}

