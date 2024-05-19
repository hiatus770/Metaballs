#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <vector> 
#include <string> 
#include <iostream> 
#include "shader.h" 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// TODO 
// -- IMplement the camera rotatoin stuff so textures appear in the right place! 
// -- Maybe do something for UI features that you will need in the future :sob: 

class Texture
{
public:
    int width, height, nrChannels;
    unsigned int texture;
    std::vector<float> vertices;
    unsigned int VBO; 
    unsigned int VAO; 
    Shader* shader; 

    glm::mat4 transform = glm::mat4(1.0f); 

    Texture(std::string location, std::vector<float> coordinates)
    {
        for(int i = 0; i < coordinates.size(); i++){
            vertices.push_back(coordinates[i]); 
        }

        // Generating texture 
        // ---------------------------------
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_set_flip_vertically_on_load(true); 
        unsigned char *data = stbi_load(location.c_str(), &width, &height, &nrChannels, 0);

        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        
        shader = new Shader("/home/hiatus/Documents/OPENGLPROJECT/BetterShaders/src/shaders/vertTexture.vs", "/home/hiatus/Documents/OPENGLPROJECT/BetterShaders/src/shaders/fragTexture.fs");

        // Setting up the texture drawing 
        glGenBuffers(1, &VBO);     

        // Create the buffer for the VBO
        glGenVertexArrays(1, &VAO);  
        glBindVertexArray(VAO); 
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW); 
        // Coordinate data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); 
        glEnableVertexAttribArray(0); 
        
        // Color Data
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1); 

        // Texture Data 
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);         
    }
    
    // Drawing the texture
    void render(){
        shader->use(); 
        shader->setMat4("transform", transform); 
        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(VAO); 
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }
};
#endif