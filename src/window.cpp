//----------------------------------------------------------

#include "window.hpp"

//----------------------------------------------------------

void windowFunction(ObjectDatabase& database, Camera& cam)
{
    // -------------------- init glfw
    if(!glfwInit())
        throw utils::GraphicsError("GLFW init failed:\n");

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
        throw utils::GraphicsError(fmt::format("GLEW init failed:\n{}", (char*)glewGetErrorString(err)));
    }
    
    // -------------------- window
    glClearColor(0.f, 1.f, 1.f, 1.f);
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // clear
        glClear(GL_COLOR_BUFFER_BIT	| GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glFlush();
    }    

    // -------------------- deinit
    glfwTerminate();
}
