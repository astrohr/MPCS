//----------------------------------------------------------

#pragma once

#include <GL/glew.h>

#include "pch.hpp"

//----------------------------------------------------------

// class used to manage gl programs (shaders)
class GLProgram{
private:
    std::string pathVertex, pathFragment;
    GLuint id;

    // loads a shader into a program
    GLuint loadShader(std::string& path, const GLuint type);

public:
    // constructor loads and compiles shaders into a program
    GLProgram(std::string vertex, std::string fragment);
    ~GLProgram();

    // activates the program
    void use() { glUseProgram(id); }
    // deactivates the currently active program
    static void useUnbind() { glUseProgram(0); }
    
};