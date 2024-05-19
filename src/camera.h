#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "object.h"

class Camera
{
public:
    glm::vec3 position; // Where the camera is 
    glm::vec3 direction; // Where the camera points  
    glm::vec3 cameraUp; // Defining the axis of movement for the camera 
    glm::mat4 projection; 

    Camera(glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 Direction = glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f), glm::mat4 Projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f)){
        position = Position;
        direction = Direction; 
        cameraUp = CameraUp; 
        projection = Projection; 
    }

    /**
     * @brief Get the View Matrix object
     * 
     * @return glm::mat4 
     */
    glm::mat4 getViewMatrix(){
        return glm::lookAt(position, position + direction, cameraUp);
    }

    /**
     * @brief Get the Projection Matrix object
     * The default is set to an FOV of 20, toDO LATER ADD CHANGABLE FOV???
     * @return glm::mat4 
     */
    glm::mat4 getProjectionMatrix(){
        return projection; 
    }
}; 

#endif