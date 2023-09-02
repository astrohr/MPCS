//----------------------------------------------------------

#include "window.hpp"

//----------------------------------------------------------


void updateInput(GLFWwindow* window, Camera& cam)
{

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    bool panUp = false, panLeft = false, panDown = false, panRight = false;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) panUp = true;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) panLeft = true;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) panDown = true;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) panRight = true;
    if (panUp || panLeft || panDown || panRight) cam.pan(panUp, panLeft, panDown, panRight);
}

void windowFunction(unsigned int W, unsigned int H, std::vector<Object>& objects, Observatory& observatory)
{
    // -------------------- init glfw
    if(!glfwInit()){
        fmt::print("GLFW init failed\n");
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, true);
    glfwWindowHint(GLFW_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(W, H, "Minor Planet Center Solver 3", NULL, NULL);

    // set the resizing function
    glfwSetFramebufferSizeCallback(window, 
        [](GLFWwindow* window, int frameBufferWidth, int frameBufferHeight) -> void {
            glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        }
    );

    glfwMakeContextCurrent(window);

    // -------------------- init glew
    glewExperimental = true;
    GLenum err = glewInit();
    if(err != GLEW_OK){
        glfwTerminate();
        fmt::print("GLEW init failed:\n{}", (char*)glewGetErrorString(err));
        return;
    }
    
    // -------------------- opengl options
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE); // dont draw the side of a vertex that cant be seen
    glCullFace(GL_BACK); // the back side cant be seen
    glFrontFace(GL_CCW); // front is where the points of a triangle are connected counterclockwise
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ------------------- set up drawing data
    std::vector<float> vertices;
    for(auto obj : objects){
        fmt::println("Log: Coords for {} - {} {} {}", obj.getName(), obj.getCoords3D().X, obj.getCoords3D().Y, obj.getCoords3D().Z);
        vertices.emplace_back(obj.getCoords3D().X);
        vertices.emplace_back(obj.getCoords3D().Y);
        vertices.emplace_back(obj.getCoords3D().Z);
    }

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // -------------------- camera
    Camera cam(W, H, 100);

    // -------------------- shader
    GLProgram program("../resources/shaders/vertex.glsl", "../resources/shaders/fragment.glsl");

    // -------------------- window
    glClearColor(0.f, 0.f, 0.f, 1.f);
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        updateInput(window, cam);

        // clear
        glClear(GL_COLOR_BUFFER_BIT	| GL_DEPTH_BUFFER_BIT);

        // draw objects
        program.setUniformMat4f("MVP", cam.getTransformation());
        glBindVertexArray(VAO);
        glPointSize(5.0f);
        glDrawArrays(GL_POINTS, 0, objects.size());

        // bureaucracy..
        glfwSwapBuffers(window);
        glFlush();
    }

    // -------------------- deinit
    glfwDestroyWindow(window);
    glfwTerminate();
}
