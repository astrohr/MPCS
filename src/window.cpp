//----------------------------------------------------------

#include "window.hpp"

//----------------------------------------------------------


void updateInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void windowFunction(ObjectDatabase& database, Camera& cam)
{
    // -------------------- init glfw
    if(!glfwInit()){
        fmt::print("GLFW init failed\n");
        return;
    }

    glfwWindowHint(GLFW_RESIZABLE, true);

    auto [W, H] = cam.getWindowSize();
    GLFWwindow* window = glfwCreateWindow(W, H, database.name().c_str(), NULL, NULL);

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

    // -------------------- shader
    GLProgram program("shader/vertex.glsl", "shader/fragment.glsl");
    program.use();

    // -------------------- window
    glClearColor(0.f, 1.f, 1.f, 1.f);
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        updateInput(window);

        // clear
        glClear(GL_COLOR_BUFFER_BIT	| GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glFlush();
    }    

    // -------------------- deinit
    glfwDestroyWindow(window);
    glfwTerminate();
}
