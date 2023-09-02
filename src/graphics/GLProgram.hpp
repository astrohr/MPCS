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

    // remember where uniforms are stored so they dont need to be fetched every time
    std::unordered_map<std::string, int> uniformLocationCache;

    // loads a shader into a program
    GLuint loadShader(std::string& path, const GLuint type);

public:
    // constructor loads and compiles shaders into a program
    GLProgram(std::string vertex, std::string fragment);
    ~GLProgram();

    int getUniformLocation(const std::string& param);

    // set shader uniform
    void setUniform3f(const std::string& param, float f1, float f2, float f3);
    void setUniformMat4f(const std::string& param, const glm::mat4& matrix);

    // activates the program
    void use() const;
    // deactivates the currently active program
    static void programUnbind() { glUseProgram(0); }
};