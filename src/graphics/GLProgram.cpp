//----------------------------------------------------------

#include "GLProgram.hpp"

//----------------------------------------------------------


GLuint GLProgram::loadShader(std::string& path, const GLuint type)
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

GLProgram::GLProgram(std::string vertex, std::string fragment)
: pathVertex(vertex), pathFragment(fragment)
{
    id = glCreateProgram();
    
    GLuint vert = loadShader(vertex, GL_VERTEX_SHADER);
    glAttachShader(id, vert);
    
    GLuint frag = loadShader(fragment, GL_FRAGMENT_SHADER);
    glAttachShader(id, frag);

    glLinkProgram(id);

    GLint success;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success){
        std::array<char, 512> infolog;
        glGetProgramInfoLog(id, 512, NULL, infolog.data());
        fmt::print("ERROR: Shader fail:\n{}\n", infolog.data());
    }

    // we dont need shaders after compilation
    glDeleteShader(vert);
    glDeleteShader(frag);
}

GLProgram::~GLProgram()
{
    glDeleteProgram(id);
}
