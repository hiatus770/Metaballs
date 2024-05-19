#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"
#include <algorithm>

class Particle 
{
public:
    glm::vec3 pos;
    glm::vec2 velocity;
    float mass = 1.0f;
    float radius = 50.0f;
    int depth; 
    std::vector<float> color = {1.0f, 1.0f, 1.0f, 1.0f};

    Particle(glm::vec3 p, glm::vec2 v, std::vector<float> Color){
        pos = p;
        velocity = v;
        color = Color; 
    }; 

    Particle(glm::vec3 p, glm::vec2 v, std::vector<float> Color, float Radius, int Depth){
        pos = p;
        velocity = v;
        color = Color;
        radius = Radius;
        depth = Depth; 
    }
};

// Handles creating, distributing, global arraying, possibly rendering too 
class ParticleHandler 
{
public: 

    std::vector<float> quadPosition = {
        -1.0f,-1.0f,
        1.0f, -1.0f, 
        1.0f,  1.0f, 
        -1.0f, 1.0f
    };

    unsigned int VAO; 
    unsigned int VBO; 
    unsigned int positionVBO; 
    unsigned int colorVBO; 
    unsigned int radiusVBO; 
    
    std::vector<Particle> particles;
    std::vector<float> positions; // Positions is generated each time from the particle global array just so we can feed it into the VBO  
    std::vector<float> colors; 
    std::vector<float> radii; 

    int particleCount = 3;
    Shader* particleShader;  

    ParticleHandler(int ParticleCount = 1){
        particleCount = ParticleCount; 
        particleShader = new Shader("/home/hiatus/Documents/2DFluidSimulator/src/shaders/circleVert.vs", "/home/hiatus/Documents/2DFluidSimulator/src/shaders/circleFrag.fs");
        
        // Temporary, jsut having one test particle right now 
        particles.push_back(Particle(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), {1.0f, 1.0f, 1.0f, 1.0f})); 
        particles.push_back(Particle(glm::vec3(100.0f, 100.0f, 0.0f), glm::vec2(0.0f, 0.0f), {0.0f, 1.0f, 1.0f, 1.0f})); 
        particles.push_back(Particle(glm::vec3(1920.0f/2, 1080.0f/2, 0.0f), glm::vec2(0.0f, 0.0f), {0.0f, 1.0f, 1.0f, 1.0f})); 

        for(int i = 0; i < 100000; i++){
            
            std::vector<float> tempColor = {(rand()%1000)/1000.0f, (float)((rand()%1000)/1000), (rand()%1000)/1000.0f, 1.0f}; 
            particles.push_back(Particle(glm::vec3(rand()%1920, rand()%1080, -(float)i), glm::vec2(0.0f, 0.0f), tempColor, rand()%100, i)); 
        
        }

        generateColorArray(); 
        generateRadiiArray(); 
        generatePositionArray(); 
        
        glGenBuffers(1, &VBO); 
        glGenBuffers(1, &colorVBO); 
        glGenBuffers(1, &radiusVBO);
        glGenBuffers(1, &positionVBO); 
        glGenVertexArrays(1, &VAO); 

        glBindVertexArray(VAO);

        // POSITION VBO 
        glBindBuffer(GL_ARRAY_BUFFER, positionVBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_DYNAMIC_COPY); 
        glEnableVertexAttribArray(1); 
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); 
        glVertexAttribDivisor(1, 1); 

        // Regular VBO 
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * quadPosition.size(), quadPosition.data(), GL_STATIC_READ); 
        glEnableVertexAttribArray(0); 
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); 

        // Setting up each particle color 
        glEnableVertexAttribArray(2); 
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colors.size(), colors.data(), GL_DYNAMIC_COPY); 
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); 
        glVertexAttribDivisor(2, 1); 

        // Setting up each radius 
        glEnableVertexAttribArray(3); 
        glBindBuffer(GL_ARRAY_BUFFER, radiusVBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * radii.size(), radii.data(), GL_DYNAMIC_COPY); 
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0); 
        glVertexAttribDivisor(3, 1); 


        // ONce we are done using we bind to 0 
        glBindBuffer(GL_ARRAY_BUFFER, 0); 

    };

    void sortParticles() {
        std::sort(particles.begin(), particles.end(), [](const Particle& a, const Particle& b) {
            return a.pos.z > b.pos.z;
        });
    }

    void render(GLenum mode = GL_LINES){
        particleShader->use();
        particleShader->setMat4("view", camera.getViewMatrix());
        particleShader->setMat4("projection", glm::ortho(0.0f, 1920.0f, 0.0f, 1080.0f, -1.0f, 1.0f)); 

        sortParticles(); 
        generateColorArray();
        generatePositionArray(); 
        generateRadiiArray(); 

        // REALLOCATING BUFFERS
        // POSITION VBO 
        glBindBuffer(GL_ARRAY_BUFFER, positionVBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_DYNAMIC_COPY); 
        glEnableVertexAttribArray(1); 
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); 
        glVertexAttribDivisor(1, 1);
        // Setting up each particle color 
        glEnableVertexAttribArray(2); 
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colors.size(), colors.data(), GL_DYNAMIC_COPY); 
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); 
        glVertexAttribDivisor(2, 1); 

        // Setting up each radius 
        glEnableVertexAttribArray(3); 
        glBindBuffer(GL_ARRAY_BUFFER, radiusVBO); 
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * radii.size(), radii.data(), GL_DYNAMIC_COPY); 
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0); 
        glVertexAttribDivisor(3, 1);


        // FINAL RENDER 
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, quadPosition.size(), particles.size()); 
        glBindVertexArray(0); 
    }

    void generateColorArray(){
        colors.clear(); 
        for(int i = 0; i < particles.size(); i++){
            colors.push_back(particles[i].color[0]);
            colors.push_back(particles[i].color[1]);
            colors.push_back(particles[i].color[2]);
            colors.push_back(particles[i].color[3]);
        }
    }

    void generatePositionArray(){
        positions.clear(); 
        for(int i = 0; i < particles.size(); i++){
            positions.push_back(particles[i].pos.x); 
            positions.push_back(particles[i].pos.y);
        }
    }

    void generateRadiiArray(){
        radii.clear(); 
        for(int i = 0; i < particles.size(); i++){
            radii.push_back(particles[i].radius); 
        }
    }
};



#endif