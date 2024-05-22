#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <utility>
#include "shader.h"
#include "object.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "texture.h"

int SRC_WIDTH = 1920;
int SRC_HEIGHT = 1080;
const int X_AMOUNT = 1920 / 5;
const int Y_AMOUNT = 1080 / 10;
const float COUNT = 384/2; 
float DELTA_L = SRC_WIDTH / COUNT; 
// float DELTA_L = 5.0f;

#include "camera.h"

float zoomLevel = 1.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::ortho((float)-SRC_WIDTH / 2, (float)SRC_WIDTH / 2, (float)-SRC_HEIGHT / 2, (float)SRC_HEIGHT / 2, -1.0f, 1.0f)); // Global Camera for the entire code thing :)
bool isDragging = false;
double lastX, lastY;
bool debug = false; // Turning this to true enables the grid and other debug messaging
bool autoscale = false; 

#include "compute.h"
#include "particle.h"
#include <time.h>

std::string HOME_DIRECTORY = "/home/hiatus/Documents/Metaballs";

// Whenever the window is changed this function is called
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// Mouse call back
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
// Processing all input here
void processInput(GLFWwindow *window);
// Scrol callback
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

std::vector<float> metaballs = {}; // These just contain the x and y coordinate of the center along with the scaling factor!
std::vector<glm::vec2> metaballsVel; 
void updateMetaballs(); 

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

int main()
{
    std::cout << "Making Window!" << std::endl;

    // ------------ OPENGL INTIALIZATION ----------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                 // The major version so x.x the first x
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // The minor version of GLFW we are using so x.x the second x
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // This means we do not use any backwards compatible features, so its a smaller set of all of OPENGL

    // Creating the window object
    GLFWwindow *window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "Balls!", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "Failed to create GLFW Window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Load GLAD function pointers so that we use the correct openGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OPENGL INITIALIZATION
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEPTH_TEST);

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glLineWidth(1.0f);

    ComputeShader computeShader(std::string(HOME_DIRECTORY + std::string("/src/shaders/compute.vs")).c_str());

    // Important vectors to track
    std::vector<float> positions;
    std::vector<float> outputPositions;

    srand(glfwGetTime());
    for (int i = 0; i < 30; i++)
    {
        metaballs.push_back(1920 / 2 - rand() % (1920));
        metaballs.push_back(1080 / 2 - rand() % (1080));
        metaballs.push_back(rand() % (5) + 15);
        metaballs.push_back(0);
        metaballsVel.push_back(glm::vec2(5 - rand()%10, 5 - rand()%10)); 
    }

    for (int i = 0; i < X_AMOUNT; i++)
    {
        for (int j = 0; j < Y_AMOUNT; j++)
        {
            positions.push_back(i * DELTA_L - SRC_WIDTH / 2);
            positions.push_back(j * DELTA_L - SRC_HEIGHT / 2);
        }
    }

    unsigned int positionSSBO;
    glGenBuffers(1, &positionSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, positionSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, positions.size() * sizeof(float), positions.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, positionSSBO);

    // Metaballs SSBO
    unsigned int metaballsSSBO;
    glGenBuffers(1, &metaballsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, metaballsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, metaballs.size() * sizeof(float), metaballs.data(), GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, metaballsSSBO);

    unsigned int outputPositionSSBO;
    glGenBuffers(1, &outputPositionSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, outputPositionSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, positions.size() * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, outputPositionSSBO);

    Shader globalShader(
        std::string(R"(#version 430 core

layout(binding = 2, std430) buffer outputpositions {
    vec2 outputPositions[];
};

uniform mat4 model; 
uniform mat4 view; 
uniform mat4 projection; 
uniform vec3 cameraPos;

void main()
{ 
    vec2 bPos = outputPositions[gl_InstanceID * 2 + gl_VertexID]; 
    gl_Position = projection * view * vec4(bPos.x, bPos.y, 0.0, 1.0);
})"),
        std::string(R"(#version 430 core
out vec4 FragColor;

uniform vec4 color = vec4(1.0, 1.0, 1.0, 1.0); 

void main()
{ 
    FragColor = color; 
})"));


    Shader normalGlobalShader(std::string(HOME_DIRECTORY + std::string("/src/shaders/regularVert.vs")).c_str(), std::string(HOME_DIRECTORY + std::string("/src/shaders/frag.fs")).c_str());
    Shader modifiedGridShader(std::string(HOME_DIRECTORY + std::string("/src/shaders/modifiedVert.vs")).c_str(),std::string(HOME_DIRECTORY + std::string("/src/shaders/frag.fs")).c_str());

    // Object metaballRenderer(&globalShader, {0.0f, 0.0f, 1080.0f, 0.0f, 1920.0f, 1080.0f});
    Object metaballRenderer(&globalShader, {0.0f, 0.0f, 1080.0f / 2, 0.0f, 1920.0f / 2, 1080.0f / 2});
    Object windowObject(&normalGlobalShader, {-1920.0f / 2, -1080.0f / 2, 1920.0f / 2, -1080.0f / 2, 1920.0f / 2, 1080.0f / 2, -1920.0f / 2, 1080.0f / 2}, {1.0f, 0.0f, 1.0f, 1.0f});

    // GRID SPACING DEBUG / DEMO CODE
    std::vector<float> grid;
    for (int i = -SRC_WIDTH / (2); i < SRC_WIDTH / 2; i += DELTA_L)
    {
        grid.push_back(i + DELTA_L / 2);
        grid.push_back(-1080.0f / 2);
        grid.push_back(i + DELTA_L / 2);
        grid.push_back(1080.0f / 2);
    }
    for (int i = -SRC_HEIGHT / (2); i < SRC_HEIGHT / 2; i += DELTA_L)
    {
        grid.push_back(-SRC_WIDTH / 2);
        grid.push_back(i + DELTA_L / 2);
        grid.push_back(SRC_WIDTH / 2);
        grid.push_back(i + DELTA_L / 2);
    }
    Object gridObject(&modifiedGridShader, grid, {0.1f, 0.1f, 0.1f, 1.0f});

    // Main Loop of the function
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::cout << "FPS: " << 1 / deltaTime << std::endl;

        // Clear the screen before we start
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.projection = glm::ortho((float)-SRC_WIDTH / (2 * zoomLevel), (float)SRC_WIDTH / (2 * zoomLevel), (float)-SRC_HEIGHT / (2 * zoomLevel), (float)SRC_HEIGHT / (2 * zoomLevel), -1.0f, 1.0f);

        // Process input call
        processInput(window);

        // Get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Convert from screen space to NDC
        xpos = xpos / SRC_WIDTH * 2.0 - 1.0;
        ypos = 1.0 - ypos / SRC_HEIGHT * 2.0;

        // Adjust for camera position and zoom
        xpos = xpos * (SRC_WIDTH / (2.0 * zoomLevel)) + camera.position.x;
        ypos = ypos * (SRC_HEIGHT / (2.0 * zoomLevel)) + camera.position.y;

        metaballs[0] = xpos;
        metaballs[1] = ypos;
        metaballs[2] = 0;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT))
        {
            metaballs[2] = 10.0f;
        }

        for(int i = 0; i < metaballs.size(); i+=4){
            float xVel = metaballsVel[(i/4)].x; 
            float yVel = metaballsVel[(i/4)].y; 
            float xPos = metaballs[i]; 
            float yPos = metaballs[i+1]; 
            
            if (xPos + xVel > SRC_WIDTH/2){
                xVel *= -1; 
            }
            if (xPos + xVel < -SRC_WIDTH/2){
                xVel *= -1; 
            }
            if (yPos + yVel > SRC_HEIGHT/2){
                yVel *= -1; 
            }
            if (yPos + yVel < -SRC_HEIGHT/2){
                yVel *= -1; 
            }
            
            metaballsVel[(i/4)].x = xVel;
            metaballsVel[(i/4)].y = yVel; 
            metaballs[i] = xPos + xVel;
            metaballs[i+1] = yPos + yVel;  
        }

        metaballs[4]++;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, metaballsSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, metaballs.size() * sizeof(float), metaballs.data(), GL_STATIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, metaballsSSBO);

        // Running compute shader
        computeShader.use();
        computeShader.setFloat("lerp", debug);
        computeShader.setFloat("delta", DELTA_L);
        computeShader.setFloat("cameraX", camera.position.x); 
        computeShader.setFloat("cameraY", camera.position.y);   
        if (autoscale){   
            computeShader.setFloat("zoom", zoomLevel); 
        } else {
            computeShader.setFloat("zoom", 1.0f); 
        }

        computeShader.dispatch();
        computeShader.wait();

        // NEW RENDERING CODE!
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, outputPositionSSBO);
        metaballRenderer.shader->use();
        metaballRenderer.shader->setVec4("color", metaballRenderer.objColor);
        metaballRenderer.shader->setMat4("model", metaballRenderer.model);
        metaballRenderer.shader->setMat4("view", camera.getViewMatrix());
        metaballRenderer.shader->setMat4("projection", camera.getProjectionMatrix());
        glBindVertexArray(metaballRenderer.VAO);
        glDrawArraysInstanced(GL_LINE_LOOP, 0, 2, X_AMOUNT * Y_AMOUNT);
        glBindVertexArray(0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        if (debug)
        {
            gridObject.render(camera.getViewMatrix(), camera.getProjectionMatrix(), GL_LINES);
            // windowObject.render(camera.getViewMatrix(), camera.getProjectionMatrix(), GL_LINES);
        }
        if (autoscale){

        }

        glfwSwapBuffers(window); // Swaps the color buffer that is used to render to during this render iteration and show it ot the output screen
        glfwPollEvents();        // Checks if any events are triggered, updates the window state andcalls the corresponding functions
    }

    glfwTerminate(); 
    return 0;
}

// This function is going to be used to resize the viewport everytime it is resized by the user
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    SRC_HEIGHT = height;
    SRC_WIDTH = width;
    glViewport(0, 0, width, height);
}

// Zooming callback
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // Calculate new zoom level
    double newZoomLevel = zoomLevel * std::exp(-yoffset / 10.0);

    // Clamp the zoom level to a range
    double minVal = 0.01;
    double maxVal = 100.0;
    newZoomLevel = std::max(minVal, newZoomLevel);
    newZoomLevel = std::min(maxVal, newZoomLevel);

    // Set the new zoom level
    if (autoscale){
        zoomLevel = newZoomLevel;
        DELTA_L = SRC_WIDTH/(COUNT * zoomLevel); 
    } else {
        zoomLevel = newZoomLevel;
        DELTA_L = SRC_WIDTH / COUNT; 
    }
}

// Mouse button callback
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        isDragging = true;
        glfwGetCursorPos(window, &lastX, &lastY);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        isDragging = false;
    }
}

// Mouse movement callback
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (isDragging)
    {
        // Calculate the mouse's offset since the last frame
        double dx = xpos - lastX;
        double dy = ypos - lastY;

        // Update the camera's position
        camera.position.x -= dx * 1 / zoomLevel;
        camera.position.y += dy * 1 / zoomLevel;

        // Update the last mouse position
        lastX = xpos;
        lastY = ypos;
    }
}

/**
 * @brief Handles all user input given the window object, currently handles player movement
 *
 * @param window
 */
bool debugKeyPressed = false;
bool autoScaleKeyPressed = false; 
void processInput(GLFWwindow *window)
{
    // // Function is used as follows player.processKeyboard(ENUM, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && debugKeyPressed == false)
    {
        debug = !debug;
        debugKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE)
    {
        debugKeyPressed = false;
    }
    if (glfwGetKey(window , GLFW_KEY_S) == GLFW_PRESS && autoScaleKeyPressed == false){
        autoscale = !autoscale; 
        autoScaleKeyPressed = true; 
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE){
        autoScaleKeyPressed = false; 
    }
}
