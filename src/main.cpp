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
const int CHUNK_SIZE = 1;
const int STARS_PER_CHUNK = 100;
const int PLANETS_PER_CHUNK = 10;
const int X_AMOUNT = 1920 / 2;
const int Y_AMOUNT = 1080 / 4;
const float DELTA_L = 4.0f;

#include "camera.h"

float zoomLevel = 1.0f;
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::ortho((float)-SRC_WIDTH / 2, (float)SRC_WIDTH / 2, (float)-SRC_HEIGHT / 2, (float)SRC_HEIGHT / 2, -1.0f, 1.0f)); // Global Camera for the entire code thing :)
bool isDragging = false;
double lastX, lastY;

#include "player.h"
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
    std::vector<float> metaballs = {-1920.0f / 2, -1080.0f / 2, 80.0f, 0.0f, 20.0f, 100.0f, 200.0f, 0.0f, 700.0f, 100.0f, 70.0f, 0.0f}; // These just contain the x and y coordinate of the center along with the scaling factor!

    srand(glfwGetTime());
    for (int i = 0; i < 20; i++)
    {
        metaballs.push_back(1920 / 2 - rand() % (1920));
        metaballs.push_back(1080 / 2 - rand() % (1080));
        metaballs.push_back(rand() % (20));
        metaballs.push_back(0);
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

    Shader globalShader(std::string(HOME_DIRECTORY + std::string("/src/shaders/vert.vs")).c_str(),std::string(HOME_DIRECTORY + std::string("/src/shaders/frag.fs")).c_str());
    Shader normalGlobalShader(std::string(HOME_DIRECTORY + std::string("/src/shaders/regularVert.vs")).c_str(), std::string(HOME_DIRECTORY + std::string("/src/shaders/frag.fs")).c_str());

    // Object testObject(&globalShader, {0.0f, 0.0f, 1080.0f, 0.0f, 1920.0f, 1080.0f});
    Object testObject(&globalShader, {0.0f, 0.0f, 1080.0f / 2, 0.0f, 1920.0f / 2, 1080.0f / 2});
    Object windowObject(&normalGlobalShader, {-1920.0f / 2, -1080.0f / 2, 1920.0f / 2, -1080.0f / 2, 1920.0f / 2, 1080.0f / 2, -1920.0f / 2, 1080.0f / 2}, {1.0f, 0.0f, 1.0f, 1.0f});

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

        // metaballs[0]++;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, metaballsSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, metaballs.size() * sizeof(float), metaballs.data(), GL_STATIC_READ);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, metaballsSSBO);

        // Running compute shader
        computeShader.use();
        computeShader.setFloat("delta", DELTA_L);
        computeShader.dispatch();
        computeShader.wait();

        // NEW RENDERING CODE!
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, outputPositionSSBO);
        testObject.shader->use();
        testObject.shader->setVec4("color", testObject.objColor);
        testObject.shader->setMat4("model", testObject.model);
        testObject.shader->setMat4("view", camera.getViewMatrix());
        testObject.shader->setMat4("projection", camera.getProjectionMatrix());
        glBindVertexArray(testObject.VAO);
        glDrawArraysInstanced(GL_LINES, 0, 2, X_AMOUNT * Y_AMOUNT);
        glBindVertexArray(0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        windowObject.render(camera.getViewMatrix(), camera.getProjectionMatrix(), GL_LINES);

        glfwSwapBuffers(window); // Swaps the color buffer that is used to render to during this render iteration and show it ot the output screen
        glfwPollEvents();        // Checks if any events are triggered, updates the window state andcalls the corresponding functions
    }

    glfwTerminate(); // Call this function to properly clean up GLFW's big mess
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
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Calculate new zoom level
    double newZoomLevel = zoomLevel * std::exp(-yoffset / 10.0);

    // Clamp the zoom level to a range
    double minVal = 0.01; 
    double maxVal = 100.0; 
    newZoomLevel = std::max(minVal, newZoomLevel);
    newZoomLevel = std::min(maxVal, newZoomLevel);

    // Set the new zoom level
    zoomLevel = newZoomLevel;
}


// Mouse button callback
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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
void processInput(GLFWwindow *window)
{
    // // Function is used as follows player.processKeyboard(ENUM, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //     player.processKeyboard(FORWARD, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //     player.processKeyboard(BACKWARD, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //     player.processKeyboard(STRAFE_LEFT, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //     player.processKeyboard(STRAFE_RIGHT, deltaTime);

    // if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    //     player.processKeyboard(PITCH_UP, deltaTime);
    // else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    //     player.processKeyboard(PITCH_DOWN, deltaTime);

    // if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    //     player.processKeyboard(YAW_RIGHT, deltaTime);
    // else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    //     player.processKeyboard(YAW_LEFT, deltaTime);

    // if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    //     player.processKeyboard(RISE, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    //     player.processKeyboard(FALL, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    //     player.processKeyboard(ROLL_RIGHT, deltaTime);
    // if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    //     player.processKeyboard(ROLL_LEFT, deltaTime);

    // if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    // {
    //     /// Reset orientation
    //     player.direction = glm::vec3(1.0f, 0.0f, 0.0f);
    //     player.localUp = glm::vec3(0.0f, 1.0f, 0.0f);
    // }

    // // Update calls
    // player.update();

    // std::cout << player.MovementSpeed << std::endl;

    // camera.position = player.getCameraPosition();
    // camera.direction = player.getCameraDirection();
    // camera.cameraUp = player.getCameraUp();
    // camera.projection =  glm::perspective(glm::radians(60.0f), (float)SRC_WIDTH/SRC_HEIGHT, 0.1f, 1000.0f);
}
