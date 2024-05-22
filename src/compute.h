#ifndef COMPUTE_H
#define COMPUTE_H

#include <glad/glad.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

class ComputeShader
{
public:
    unsigned int ID;
    ComputeShader(const char *path)
    {
        std::string shaderCode;
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try
        {
            shaderFile.open(path);
            std::cout << "Opening Compute Shader Path" << std::endl;

            std::stringstream shaderStream;

            shaderStream << shaderFile.rdbuf();
            shaderFile.close();

            shaderCode = shaderStream.str();
        }
        catch (std::ifstream::failure &e)
        {
            std::cout << "COMPUTE SHADER NOT READ PROPERLY: " << e.what() << std::endl;
        }
        const char *cShaderCode = shaderCode.c_str();
        unsigned int compute;
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(compute);
    }

    ComputeShader(std::string computeSource)
    {
        const char *cShaderCode = computeSource.c_str();
        unsigned int compute;
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        checkCompileErrors(compute, "COMPUTE");

        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");

        glDeleteShader(compute);
    }

    void use()
    {
        glUseProgram(ID);
    }
    void dispatch()
    {
        // just keep it simple, 2d work group
        glDispatchCompute(X_AMOUNT * Y_AMOUNT, 1, 1);
    }
    void wait()
    {
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }

    void setFloat(const std::string &name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                          << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};

#endif