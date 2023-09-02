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

    this->use();
}

GLProgram::~GLProgram()
{
    glDeleteProgram(id);
    this->programUnbind();
}


int GLProgram::getUniformLocation(const std::string& param)
{
    if (uniformLocationCache.find(param) != uniformLocationCache.end()) return uniformLocationCache[param];

    int location = glGetUniformLocation(id, param.c_str());
    
    if (location == -1) fmt::println("Warning: uniform {} doesnt exist", param);
    
    uniformLocationCache[param] = location;
    return location;
}


void GLProgram::setUniform3f(const std::string& param, float f1, float f2, float f3)
{
    this->use();
    glUniform3f(this->getUniformLocation(param), f1, f2, f3);
}

void GLProgram::setUniformMat4f(const std::string& param, const glm::mat4& matrix)
{
    this->use();
    glUniformMatrix4fv(this->getUniformLocation(param), 1, false, &matrix[0][0]);
}

void GLProgram::use() const
{
    glUseProgram(id); 
}