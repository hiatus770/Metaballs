#ifndef PLAYER_H
#define PLAYER_H

#include <vector>
#include "shader.h"
#include "object.h"
#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


enum Player_Movement
{
    FORWARD,
    BACKWARD,
    STRAFE_LEFT,
    STRAFE_RIGHT,
    ROLL_LEFT,
    ROLL_RIGHT,
    RISE,
    FALL,
    PITCH_UP,
    PITCH_DOWN,
    YAW_LEFT,
    YAW_RIGHT
};

const glm::vec3 cameraOffset = glm::vec3(-2.0f, 0.5f, 0.0f);

// const glm::vec3 cameraOffset = glm::vec3(0.0f, 0.0f, 0.0f);
class Player
{
public:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 localUp;
    std::vector<float> vertices;
    std::vector<float> color;
    Object *playerObj;
    Shader *shader;
    float velocity; 
    float rotateVelocity; 
    float RotateMovementSpeed = 50;
    float MovementSpeed = 2.5;

    Player(std::vector<float> Vertices, glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 Direction = glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3 LocalUp = glm::vec3(0.0f, 1.0f, 0.0f))
    {
        position = Position;
        direction = Direction;
        localUp = LocalUp;
        vertices = Vertices;
    }

    void createPlayerObject(Shader *gShader)
    {
        playerObj = new Object(gShader, vertices, {1.0f, 1.0f, 0.0f, 1.0f});
        playerObj->matrixTransform(glm::translate(playerObj->model, position));
    }

    void processKeyboard(Player_Movement dir, float deltaTime = 0.1)
    {
        glm::vec3 right = glm::normalize(glm::cross(direction, localUp)); // Assuming world up is y-axis

        velocity = MovementSpeed * deltaTime;
        rotateVelocity = RotateMovementSpeed * deltaTime;

        if (dir == FORWARD)
            // position += velocity * direction;
            MovementSpeed += 0.25; 
        else if (dir == BACKWARD)
            MovementSpeed -= 0.25; 
            // position -= velocity * direction;
        if (dir == ROLL_LEFT)
            localUp = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(1.0f * rotateVelocity), direction)) * localUp;
        if (dir == ROLL_RIGHT)
            localUp = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * rotateVelocity), direction)) * localUp;
        if (dir == PITCH_UP)
        {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(1.0f * rotateVelocity), right)) * direction;
            localUp = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(1.0f * rotateVelocity), right)) * localUp;
        }
        if (dir == PITCH_DOWN)
        {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * rotateVelocity), right)) * direction;
            localUp = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * rotateVelocity), right)) * localUp;
        }

        if (dir == YAW_RIGHT)
        {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(-1.0f * rotateVelocity), localUp)) * direction;
        }

        if (dir == YAW_LEFT)
        {
            direction = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(1.0f * rotateVelocity), localUp)) * direction;
        }

        // Strafing keys
        if (dir == STRAFE_LEFT)
        {
            position -= glm::normalize(glm::cross(direction, localUp)) * fabs(7*deltaTime);
        }
        if (dir == STRAFE_RIGHT)
        {
            position += glm::normalize(glm::cross(direction, localUp)) * fabs(7*deltaTime);
        }

        // Rise and Fall keys
        if (dir == RISE)
        {
            position += glm::normalize(localUp) * velocity;
        }
        if (dir == FALL)
        {
            position -= glm::normalize(localUp) * velocity;
        }


        
    }

    void update(){
        position += velocity * direction;
    }

    glm::vec3 getCameraOffset()
    {
        // return glm::vec3(0); // Actually use the vector components this time please
        return glm::normalize(localUp) * cameraOffset.y + glm::normalize(direction) * cameraOffset.x + glm::normalize(glm::cross(localUp, direction)) * cameraOffset.z;
    }

    glm::vec3 getCameraDirection()
    {
        // return ( position + glm::normalize(direction*-3.0f)) - (position + getCameraOffset());
        return direction;
    }

    glm::vec3 getCameraPosition()
    {
        return position + getCameraOffset();
    }

    glm::vec3 getCameraUp()
    {
        return localUp; // getCameraOffset();
    }

    /**
     * @brief Renders the player ship in front of the camera
     *
     */
    void render()
    {

        glm::quat orientation = glm::quatLookAt(direction, localUp);

        // Convert the quaternion to a rotation matrix
        glm::mat4 rotation = glm::mat4_cast(orientation);

        playerObj->model = (glm::translate(glm::mat4(1.0f), position)) * rotation;

        playerObj->render(camera.getViewMatrix(), camera.getProjectionMatrix(), GL_LINES);
    }

    void overrideRender(glm::mat4 viewMatrix, glm::mat4 projectionMatrix){}

    // Return camera nonsense for the position of the camera, dont actually store the camrea here make a global one!
};

#endif