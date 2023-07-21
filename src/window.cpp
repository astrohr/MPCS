//----------------------------------------------------------

#include "window.hpp"

//----------------------------------------------------------



GLuint loadShader(std::string& path, GLuint type)
{
    std::ifstream code(path);

    if (!code.is_open())
        fmt::print("No shader at\n{}\n", path);
    
    std::string src = "", temp = "";
    while(std::getline(code, temp)) src += temp + '\n';

    GLuint shader = glCreateShader(type);
    if (shader == GL_INVALID_ENUM)
        fmt::print("shader at\n{}\nis called with a wrong type\n", path);

    const GLchar* shaderSrc = src.c_str();
    glShaderSource(shader, 1, &shaderSrc, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success){
        std::array<char, 512> infolog;
        glGetShaderInfoLog(shader, 512, NULL, infolog.data());
        fmt::print("ERROR: Shader fail:\n{}\n", infolog.data());
    }

    code.close();
    return shader;
}

GLuint newProgram(std::string&& vertex, std::string&& fragment)
{
    GLuint program = glCreateProgram();
    
    GLuint vert = loadShader(vertex, GL_VERTEX_SHADER);
    glAttachShader(program, vert);
    
    GLuint frag = loadShader(fragment, GL_FRAGMENT_SHADER);
    glAttachShader(program, frag);

    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success){
        std::array<char, 512> infolog;
        glGetProgramInfoLog(program, 512, NULL, infolog.data());
        fmt::print("ERROR: Shader fail:\n{}\n", infolog.data());
    }

    glUseProgram(0);
    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

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

    GLuint program = newProgram("shader/vertex.glsl", "shader/fragment.glsl");

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
    glDeleteProgram(program);
    glfwTerminate();
}
