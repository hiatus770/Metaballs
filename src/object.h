#ifndef OBJECT_H
#define OBJECT_H

#include <glad/glad.h>
#include "shader.h" 
#include <vector>
#include <iostream> 

class Object
{
public:
    std::vector<float> vertices; // The list of vertices for all the lines
    unsigned int VBO; 
    unsigned int VAO; 
    std::vector<float> objColor; 
    glm::mat4 model = glm::mat4(1.0f); 

    Shader* shader; 
    
    Object(Shader* gShader, std::vector<float> v = {}, 
           std::vector<float> color = {1.0, 1.0, 1.0, 1.0})
    {
        shader = gShader; 

        for(int i = 0; i < v.size(); i++){
            vertices.push_back(v[i]); 
        }

        for(int i = 0; i < color.size(); i++){
            objColor.push_back(color[i]); 
        }

        // Gen VAO and VBO 
        glGenBuffers(1, &VBO); 
        glGenVertexArrays(1, &VAO); 

        // Bind VAO 
        glBindVertexArray(VAO); 
        glBindBuffer(GL_ARRAY_BUFFER, VBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW); 
        // Assumes we are doing 3 floats 
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); 
        // Enable this attribute now in the shader 
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    }

    /**
     * @brief Sets the model matrix to mat4 transformation, tbh idk why this is needed 
     * 
     * @param transformation 
     */
    void matrixTransform(glm::mat4 transformation){ 
        model = transformation; 
    }

    /**
     * @brief Renders a given object using a projection matrix, view matrix and other stuff 
     * 
     * @param view 
     * @param projection 
     * @param mode 
     */
    void render(glm::mat4 view, glm::mat4 projection, GLenum mode = GL_TRIANGLE_FAN){
        shader->use(); 
        glBindBuffer(GL_ARRAY_BUFFER, VBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); 
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0); 
        shader->setVec4("color", objColor); 
        shader->setMat4("model", model); 
        shader->setMat4("view", view); 
        shader->setMat4("projection", projection); 
        glBindVertexArray(VAO);
        glDrawArrays(mode, 0, vertices.size()); 
        glBindVertexArray(0); 
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
    }

};
#endif 