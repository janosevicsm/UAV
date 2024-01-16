// Autor: Marko Janosevic - SV 46/2020
// Opis: Projektni zadatak iz predmeta Racunarska Grafika
// Naziv: UAV Control Interface - Bespilotnik

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "model.hpp"

static unsigned loadImageToTexture(const char* filePath);
void drawMapTexture(Shader shader, unsigned int texture, unsigned int specularTexture, unsigned int VAO);
void drawBackgroundTexture(Shader shader, unsigned int texture, unsigned int VAO);
void drawActiveIndicator(Shader shader, unsigned int VAO, unsigned int count, unsigned int newColLoc, bool isActive, bool isDestroyed);
void drawDroneMinimap(Shader shader, unsigned int VAO, unsigned int count, glm::mat4 model, float y);
void drawBatteryBar(Shader shader, unsigned int VAO, unsigned int count, unsigned int barLvl, float batteryPercentage);
void drawForbiddenZone(Shader shader, unsigned int VAO, unsigned int count);

const unsigned int wWidth = 1600;
const unsigned int wHeight = 800;

const int CRES = 30;
const float pointSize = 5.0f;
const int NUMBER_OF_BUFFERS = 10;
const int mapTextureIndex = 0;
const int drone1ActiveIndicatorIndex = 1;
const int drone2ActiveIndicatorIndex = 2;
const int noSignalIndex = 3;
const int drone1MinimapIndex = 4;
const int drone2MinimapIndex = 5;
const int batteryBar1Index = 6;
const int batteryBar2Index = 7;
const int signatureIndex = 8;
const int forbiddenZoneIndex = 9;

float cameraEyeX = 0.0f;
float cameraEyeY = 4.0f;
float cameraEyeZ = 4.0f;
float cameraCenterX = 0.0f;
float cameraCenterY = 2.0f;
float cameraCenterZ = 0.0f;

const float droneSize = 0.775;

float drone1X = -0.5;
float drone1Y = 2;
float drone1Z = 0;
float drone1Angle = 90;

float drone2X = 0.5;
float drone2Y = 2;
float drone2Z = 0;
float drone2Angle = 90;


const float movementChange = 0.0005;
const float angleChange = 0.1;
const float batteryChange = 0.001;


bool isDrone1Active = true;
bool isDrone2Active = true;
bool isDrone1Destroyed = false;
bool isDrone2Destroyed = false;
bool isDrone1CameraActive = true;
bool isDrone2CameraActive = true;

float drone1BatteryPercentage = 100;
float drone2BatteryPercentage = 100;

const float forbiddenZoneX = -1.8;
const float forbiddenZoneY = -3.0;

int main(void)
{
    #pragma region Setup

    if (!glfwInit())
    {
        std::cout << "GLFW fail!\n" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int screenWidth = mode->width;
    int screenHeight = mode->height;

    GLFWwindow* window = glfwCreateWindow(wWidth, wHeight, "UAV", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Window fail!\n" << std::endl;
        glfwTerminate();
        return -2;
    }

    int windowPosX = (screenWidth - wWidth) / 2;
    int windowPosY = (screenHeight - wHeight) / 2;
    glfwSetWindowPos(window, windowPosX, windowPosY);

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW fail! :(\n" << std::endl;
        return -3;
    }

    unsigned int VAO[NUMBER_OF_BUFFERS];
    glGenVertexArrays(NUMBER_OF_BUFFERS, VAO);
    unsigned int VBO[NUMBER_OF_BUFFERS];
    glGenBuffers(NUMBER_OF_BUFFERS, VBO);
    unsigned int EBO[NUMBER_OF_BUFFERS];
    glGenBuffers(NUMBER_OF_BUFFERS, EBO);

    #pragma endregion

    #pragma region Map Texture

    float mapVertices[] =
    {
        -5.0,  0.0,  -5.0,    0.0, 1.0, 0.0,    0.0, 1.0,
        -5.0,  0.0,  5.0,     0.0, 1.0, 0.0,    0.0, 0.0,
         5.0,  0.0,  -5.0,    0.0, 1.0, 0.0,    1.0, 1.0,
         5.0,  0.0,  5.0,     0.0, 1.0, 0.0,    1.0, 0.0
    };

    unsigned int mapIndecies[] =
    {
        0, 1, 3,
        3, 2, 0
    };

    unsigned int mapStride = (3 + 3 + 2) * sizeof(float);

    glBindVertexArray(VAO[mapTextureIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[mapTextureIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(mapVertices), mapVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, mapStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, mapStride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, mapStride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[mapTextureIndex]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(mapIndecies), mapIndecies, GL_STATIC_DRAW);

    glBindVertexArray(0);

    unsigned mapTexture = loadImageToTexture("res/images/map.png");
    glBindTexture(GL_TEXTURE_2D, mapTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned specularMapTexture = loadImageToTexture("res/images/specular_map.png");
    glBindTexture(GL_TEXTURE_2D, specularMapTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    unsigned minimapTexture = loadImageToTexture("res/images/minimap.png");
    glBindTexture(GL_TEXTURE_2D, minimapTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
   
    #pragma endregion

    #pragma region Active Indicators

    float drone1IndicatorVertices[CRES * 2 + 4];
    float drone2IndicatorVertices[CRES * 2 + 4];
    float indicatorRadius = 0.1;

    drone1IndicatorVertices[0] = -0.85;
    drone1IndicatorVertices[1] = 0.85;
    drone2IndicatorVertices[0] = -0.85;
    drone2IndicatorVertices[1] = 0.6;

    int i;
    for (i = 0; i <= CRES; i++)
    {
        drone1IndicatorVertices[2 + 2 * i] = drone1IndicatorVertices[0] + indicatorRadius * cos((3.141592 / 180) * (i * 360 / CRES));
        drone1IndicatorVertices[2 + 2 * i + 1] = drone1IndicatorVertices[1] + indicatorRadius * sin((3.141592 / 180) * (i * 360 / CRES));
        drone2IndicatorVertices[2 + 2 * i] = drone2IndicatorVertices[0] + indicatorRadius * cos((3.141592 / 180) * (i * 360 / CRES));
        drone2IndicatorVertices[2 + 2 * i + 1] = drone2IndicatorVertices[1] + indicatorRadius * sin((3.141592 / 180) * (i * 360 / CRES));
    }

    unsigned int indicatorStride = 2 * sizeof(float);

    glBindVertexArray(VAO[drone1ActiveIndicatorIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[drone1ActiveIndicatorIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(drone1IndicatorVertices), drone1IndicatorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, indicatorStride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glBindVertexArray(VAO[drone2ActiveIndicatorIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[drone2ActiveIndicatorIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(drone2IndicatorVertices), drone2IndicatorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, indicatorStride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    Shader activeIndicatorsShader("circle.vert", "indicator.frag");
    unsigned int newColLoc = glGetUniformLocation(activeIndicatorsShader.ID, "newCol");

    #pragma endregion

    #pragma region Signature

    float signatureVertices[] =
    {
        -0.7, 1.6,  0.0, 1.0,
        -0.7, 0.6, 0.0, 0.0,
        0.3, 1.6,  1.0, 1.0,
        0.3, 0.6,  1.0, 0.0
    };

    unsigned int signatureIndecies[] =
    {
        0, 1, 3,
        3, 2, 0
    };

    unsigned int signatureStride = (2 + 2) * sizeof(float);

    glBindVertexArray(VAO[signatureIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[signatureIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(signatureVertices), signatureVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, signatureStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[signatureIndex]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(signatureIndecies), signatureIndecies, GL_STATIC_DRAW);

    glBindVertexArray(0);

    Shader signatureShader("signature.vert", "signature.frag");
    unsigned signatureTexture = loadImageToTexture("res/images/signature.png");

    glBindTexture(GL_TEXTURE_2D, signatureTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    signatureShader.use();
    signatureShader.setInt("uTex", 0);

    #pragma endregion

    #pragma region No Signal Texture

    float noSignalVertices[] =
    {
        -1.0, 1.0,  0.0, 1.0,
        -1.0, -1.0, 0.0, 0.0,
        1.0, 1.0,  1.0, 1.0,
        1.0, -1.0,  1.0, 0.0
    };

    unsigned int noSignalIndecies[] =
    {
        0, 1, 2,
        1, 2, 3
    };

    unsigned int noSignalStride = (2 + 2) * sizeof(float);

    glBindVertexArray(VAO[noSignalIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[noSignalIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(noSignalVertices), noSignalVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, noSignalStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, noSignalStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[noSignalIndex]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(noSignalIndecies), noSignalIndecies, GL_STATIC_DRAW);

    glBindVertexArray(0);

    Shader noSignalShader("signature.vert", "signature.frag");
    unsigned noSignalTexture = loadImageToTexture("res/images/noSignal.jpg");

    glBindTexture(GL_TEXTURE_2D, noSignalTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    noSignalShader.use();
    noSignalShader.setInt("uTex", 0);

    #pragma endregion

    #pragma region Drones - Minimap

    float drone1MinimapVertices[2];
    float drone2MinimapVertices[2];

    drone1MinimapVertices[0] = drone1X / 5;
    drone1MinimapVertices[1] = drone1Z / 5;
    drone2MinimapVertices[0] = drone2X / 5;
    drone2MinimapVertices[1] = drone2Z / 5;

    unsigned int droneMinimapStride = 2 * sizeof(float);

    glBindVertexArray(VAO[drone1MinimapIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[drone1MinimapIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(drone1MinimapVertices), drone1MinimapVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, droneMinimapStride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    glBindVertexArray(VAO[drone2MinimapIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[drone2MinimapIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(drone2MinimapVertices), drone2MinimapVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, droneMinimapStride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    Shader droneMinimapShader("droneMinimap.vert", "droneMinimap.frag");

    #pragma endregion

    #pragma region Battery Bar

    float batteryBar1Vertices[] = {
        -1.0, -0.6,     0.3647, 1.0, 0.1412, 1.0,
        -1.0, -0.4,     0.3647, 1.0, 0.1412, 1.0,
        0.0, -0.4,     0.3647, 1.0, 0.1412, 1.0,
        0.0, -0.6,     0.3647, 1.0, 0.1412, 1.0
    };

    float batteryBar2Vertices[] = {
        -1.0, -0.9,     0.3647, 1.0, 0.1412, 1.0,
        -1.0, -0.7,     0.3647, 1.0, 0.1412, 1.0,
        0.0, -0.7,     0.3647, 1.0, 0.1412, 1.0,
        0.0, -0.9,     0.3647, 1.0, 0.1412, 1.0
    };

    unsigned int batteryBarStride = 6 * sizeof(float);

    glBindVertexArray(VAO[batteryBar1Index]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[batteryBar1Index]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(batteryBar1Vertices), batteryBar1Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, batteryBarStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, batteryBarStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(VAO[batteryBar2Index]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[batteryBar2Index]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(batteryBar2Vertices), batteryBar2Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, batteryBarStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, batteryBarStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader batteryBarShader("bar.vert", "bar.frag");
    unsigned int batteryLvl = glGetUniformLocation(batteryBarShader.ID, "batteryLevel");

    #pragma endregion

    #pragma region ForbiddenZone

    float forbiddenZoneVertices[CRES * 2 + 4];
    float forbiddenZoneRadius = 1.8;

    forbiddenZoneVertices[0] = forbiddenZoneX;
    forbiddenZoneVertices[1] = -forbiddenZoneY;

    int f;
    for (f = 0; f <= CRES; f++)
    {
        forbiddenZoneVertices[2 + 2 * f] = forbiddenZoneVertices[0] + forbiddenZoneRadius * cos((3.141592 / 180) * (f * 360 / CRES));
        forbiddenZoneVertices[2 + 2 * f + 1] = forbiddenZoneVertices[1] + forbiddenZoneRadius * sin((3.141592 / 180) * (f * 360 / CRES));
    }

    unsigned int forbiddenZoneStride = 2 * sizeof(float);

    glBindVertexArray(VAO[forbiddenZoneIndex]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[forbiddenZoneIndex]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(forbiddenZoneVertices), forbiddenZoneVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, forbiddenZoneStride, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    Shader forbiddenZoneShader("forbiddenZone.vert", "forbiddenZone.frag");

    #pragma endregion

    Model drone1("res/drone/Drone_obj.obj");
    Model drone2("res/drone/Drone_obj.obj");
    Model redLight("res/light/lightRed.obj");
    Model greenLight("res/light/lightGreen.obj");
    Model whiteLight("res/light/lightWhite.obj");

    Shader unifiedShader("basic.vert", "basic.frag");
    
    #pragma region Light Settings

    unifiedShader.use();
    unifiedShader.setInt("material.diffuse", 0);
    unifiedShader.setInt("material.specular", 1);
    unifiedShader.setFloat("material.shininess", 32.0f);

    glm::vec3 droneLightAmbient = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 droneLightDiffuse = glm::vec3(10.5, 10.5, 10.5);
    glm::vec3 droneLightSpecular = glm::vec3(5.0, 5.0, 5.0);

    float constant = 1.0f;
    float linear = 4.0f;
    float quadratic = 2.0f;
    
    // Drone 1 Red Light
    unifiedShader.setVec3("pointLights[0].color", 1.0f, 0.0f, 0.0f);
    unifiedShader.setVec3("pointLights[0].ambient", droneLightAmbient);
    unifiedShader.setVec3("pointLights[0].diffuse", droneLightDiffuse);
    unifiedShader.setVec3("pointLights[0].specular", droneLightSpecular);
    unifiedShader.setFloat("pointLights[0].constant", constant);
    unifiedShader.setFloat("pointLights[0].linear", linear);
    unifiedShader.setFloat("pointLights[0].quadratic", quadratic);

    // Drone 1 Green Light
    unifiedShader.setVec3("pointLights[1].color", 0.0f, 1.0f, 0.0f);
    unifiedShader.setVec3("pointLights[1].ambient", droneLightAmbient);
    unifiedShader.setVec3("pointLights[1].diffuse", droneLightDiffuse);
    unifiedShader.setVec3("pointLights[1].specular", droneLightSpecular);
    unifiedShader.setFloat("pointLights[1].constant", constant);
    unifiedShader.setFloat("pointLights[1].linear", linear);
    unifiedShader.setFloat("pointLights[1].quadratic", quadratic);

    // Drone 1 White Light
    unifiedShader.setVec3("pointLights[2].color", 0.0f, 0.0f, 0.0f);
    unifiedShader.setVec3("pointLights[2].ambient", droneLightAmbient);
    unifiedShader.setVec3("pointLights[2].diffuse", droneLightDiffuse);
    unifiedShader.setVec3("pointLights[2].specular", droneLightSpecular);
    unifiedShader.setFloat("pointLights[2].constant", constant);
    unifiedShader.setFloat("pointLights[2].linear", linear);
    unifiedShader.setFloat("pointLights[2].quadratic", quadratic);

    // Drone 2 Red Light
    unifiedShader.setVec3("pointLights[3].color", 1.0f, 0.0f, 0.0f);
    unifiedShader.setVec3("pointLights[3].ambient", droneLightAmbient);
    unifiedShader.setVec3("pointLights[3].diffuse", droneLightDiffuse);
    unifiedShader.setVec3("pointLights[3].specular", droneLightSpecular);
    unifiedShader.setFloat("pointLights[3].constant", constant);
    unifiedShader.setFloat("pointLights[3].linear", linear);
    unifiedShader.setFloat("pointLights[3].quadratic", quadratic);

    // Drone 2 Green Light
    unifiedShader.setVec3("pointLights[4].color", 0.0f, 1.0f, 0.0f);
    unifiedShader.setVec3("pointLights[4].ambient", droneLightAmbient);
    unifiedShader.setVec3("pointLights[4].diffuse", droneLightDiffuse);
    unifiedShader.setVec3("pointLights[4].specular", droneLightSpecular);
    unifiedShader.setFloat("pointLights[4].constant", constant);
    unifiedShader.setFloat("pointLights[4].linear", linear);
    unifiedShader.setFloat("pointLights[4].quadratic", quadratic);

    // Drone 2 White Light
    unifiedShader.setVec3("pointLights[5].color", 0.0f, 0.0f, 0.0f);
    unifiedShader.setVec3("pointLights[5].ambient", droneLightAmbient);
    unifiedShader.setVec3("pointLights[5].diffuse", droneLightDiffuse);
    unifiedShader.setVec3("pointLights[5].specular", droneLightSpecular);
    unifiedShader.setFloat("pointLights[5].constant", constant);
    unifiedShader.setFloat("pointLights[5].linear", linear);
    unifiedShader.setFloat("pointLights[5].quadratic", quadratic);

    // Main Light
    unifiedShader.setVec3("pointLights[6].position", 2.4f, 0.05f, -1.8f);
    unifiedShader.setVec3("pointLights[6].color", 1.0f, 1.0f, 1.0f);
    unifiedShader.setVec3("pointLights[6].ambient", 0.2f, 0.2f, 0.2f);
    unifiedShader.setVec3("pointLights[6].diffuse", 2.0f, 2.0f, 2.0f);
    unifiedShader.setVec3("pointLights[6].specular", 2.0f, 2.0f, 2.0f);
    unifiedShader.setFloat("pointLights[6].constant", 1.0f);
    unifiedShader.setFloat("pointLights[6].linear", 0.09f);
    unifiedShader.setFloat("pointLights[6].quadratic", 0.032f);

    unifiedShader.setVec3("pointLights[7].position", 0.0, 5.0f, 0.0);
    unifiedShader.setVec3("pointLights[7].color", 1.0f, 1.0f, 1.0f);
    unifiedShader.setVec3("pointLights[7].ambient", 0.2f, 0.2f, 0.2f);
    unifiedShader.setVec3("pointLights[7].diffuse", 2.0f, 2.0f, 2.0f);
    unifiedShader.setVec3("pointLights[7].specular", 2.0f, 2.0f, 2.0f);
    unifiedShader.setFloat("pointLights[7].constant", 1.0f);
    unifiedShader.setFloat("pointLights[7].linear", 0.09f);
    unifiedShader.setFloat("pointLights[7].quadratic", 0.032f);

    #pragma endregion

    #pragma region Pre-render settings
    
    unifiedShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    unifiedShader.setMat4("uP", projection);
    glm::mat4 view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
    unifiedShader.setMat4("uV", view);

    glm::mat4 model1 = glm::mat4(1.0f);
    glm::mat4 model2 = glm::mat4(1.0f);
    glm::mat4 mapModel = glm::mat4(1.0f);
    glm::mat4 redLightModel1 = glm::mat4(1.0f);
    glm::mat4 greenLightModel1 = glm::mat4(1.0f);
    glm::mat4 whiteLightModel1 = glm::mat4(1.0f);
    glm::mat4 redLightModel2 = glm::mat4(1.0f);
    glm::mat4 greenLightModel2 = glm::mat4(1.0f);
    glm::mat4 whiteLightModel2 = glm::mat4(1.0f);

    model1 = glm::translate(model1, glm::vec3(drone1X, drone1Y, drone1Z));
    model2 = glm::translate(model2, glm::vec3(drone2X, drone2Y, drone2Z));

    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));

    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));

    float drone1ViewCenterX;
    float drone1ViewCenterZ;
    float drone2ViewCenterX;
    float drone2ViewCenterZ;

    float minimapWidth = wWidth / 4;
    float minimapHeight = wHeight / 2;
    droneMinimapShader.use();
    glm::mat4 minimapProjection = glm::ortho(-minimapWidth / 80.0f, minimapWidth / 80.0f, -minimapHeight / 80.0f, minimapHeight / 80.0f, 0.1f, 100.0f);
    droneMinimapShader.setMat4("uP", minimapProjection);
    glm::mat4 minimapView = glm::lookAt(glm::vec3(0.0, 10.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0f, 0.0f, -1.0f));
    droneMinimapShader.setMat4("uV", minimapView);

    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    #pragma endregion

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        unifiedShader.setBool("isMinimap", false);

        #pragma region Camera Movement

        // Translation

        if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
            cameraEyeZ -=  8 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }
        if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
            cameraEyeZ += 8 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }
        if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
            cameraEyeX -= 8 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }
        if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
            cameraEyeX += 8 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }

        // Rotation

        if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            cameraCenterY -= 10 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }
        if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            cameraCenterY += 10 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }
        if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            cameraCenterX -= 10 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }
        if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            cameraCenterX += 10 * movementChange;
            view = glm::lookAt(glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ), glm::vec3(cameraCenterX, cameraCenterY, cameraCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
            unifiedShader.use();
            unifiedShader.setMat4("uV", view);
        }

        #pragma endregion

        #pragma region Drone Activation

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            isDrone1Active = true;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            isDrone2Active = true;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            isDrone1Active = false;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            isDrone2Active = false;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
            isDrone1CameraActive = true;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
            isDrone2CameraActive = true;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            isDrone1CameraActive = false;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            isDrone2CameraActive = false;

        #pragma endregion

        #pragma region Drone Movement

        if (!isDrone1Destroyed) {
            if (isDrone1Active) {
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    drone1X += movementChange * sin(drone1Angle * 0.0174533);
                    drone1Z += movementChange * cos(drone1Angle * 0.0174533);
                    model1 = glm::translate(model1, glm::vec3(movementChange, 0.0f, 0.0f));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    drone1X -= movementChange * sin(drone1Angle * 0.0174533);
                    drone1Z -= movementChange * cos(drone1Angle * 0.0174533);
                    model1 = glm::translate(model1, glm::vec3(-movementChange, 0.0f, 0.0f));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    drone1X -= movementChange * cos(drone1Angle * 0.0174533);
                    drone1Z += movementChange * sin(drone1Angle * 0.0174533);
                    model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, movementChange));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    drone1X += movementChange * cos(drone1Angle * 0.0174533);
                    drone1Z -= movementChange * sin(drone1Angle * 0.0174533);
                    model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, -movementChange));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                {
                    drone1Y += movementChange;
                    model1 = glm::translate(model1, glm::vec3(0.0f, movementChange, 0.0f));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                {
                    drone1Y -= movementChange;
                    model1 = glm::translate(model1, glm::vec3(0.0f, -movementChange, 0.0f));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    drone1Angle -= angleChange;
                    model1 = glm::rotate(model1, glm::radians(-angleChange), glm::vec3(0.0f, 1.0f, 0.0f));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    drone1Angle += angleChange;
                    model1 = glm::rotate(model1, glm::radians(angleChange), glm::vec3(0.0f, 1.0f, 0.0f));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
            }
            else {
                if (drone1Y > 0.07) {
                    drone1Y -= movementChange / 2;
                    model1 = glm::translate(model1, glm::vec3(0.0f, -movementChange / 2, 0.0f));
                    redLightModel1 = glm::translate(model1, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel1 = glm::scale(redLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel1 = glm::translate(model1, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel1 = glm::scale(greenLightModel1, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel1 = glm::translate(model1, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel1 = glm::scale(whiteLightModel1, glm::vec3(0.2, 0.2, 0.2));
                }
            }
        }

        if (!isDrone2Destroyed) {
            if (isDrone2Active) {
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                {
                    drone2X += movementChange * sin(drone2Angle * 0.0174533);
                    drone2Z += movementChange * cos(drone2Angle * 0.0174533);
                    model2 = glm::translate(model2, glm::vec3(movementChange, 0.0f, 0.0f));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                {
                    drone2X -= movementChange * sin(drone2Angle * 0.0174533);
                    drone2Z -= movementChange * cos(drone2Angle * 0.0174533);
                    model2 = glm::translate(model2, glm::vec3(-movementChange, 0.0f, 0.0f));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                {
                    drone2X -= movementChange * cos(drone2Angle * 0.0174533);
                    drone2Z += movementChange * sin(drone2Angle * 0.0174533);
                    model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, movementChange));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                {
                    drone2X += movementChange * cos(drone2Angle * 0.0174533);
                    drone2Z -= movementChange * sin(drone2Angle * 0.0174533);
                    model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, -movementChange));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                {
                    drone2Y += movementChange;
                    model2 = glm::translate(model2, glm::vec3(0.0f, movementChange, 0.0f));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                {
                    drone2Y -= movementChange;
                    model2 = glm::translate(model2, glm::vec3(0.0f, -movementChange, 0.0f));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                {
                    drone2Angle -= angleChange;
                    model2 = glm::rotate(model2, glm::radians(-angleChange), glm::vec3(0.0f, 1.0f, 0.0f));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                {
                    drone2Angle += angleChange;
                    model2 = glm::rotate(model2, glm::radians(angleChange), glm::vec3(0.0f, 1.0f, 0.0f));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
            }
            else {
                if (drone2Y > 0.07) {
                    drone2Y -= movementChange / 2;
                    model2 = glm::translate(model2, glm::vec3(0.0f, -movementChange / 2, 0.0f));
                    redLightModel2 = glm::translate(model2, glm::vec3(-droneSize / 2, 0.0, 0.0));
                    redLightModel2 = glm::scale(redLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    greenLightModel2 = glm::translate(model2, glm::vec3(droneSize / 2, 0.0, 0.0));
                    greenLightModel2 = glm::scale(greenLightModel2, glm::vec3(0.2, 0.2, 0.2));
                    whiteLightModel2 = glm::translate(model2, glm::vec3(0.0, 0.0, droneSize / 5.5));
                    whiteLightModel2 = glm::scale(whiteLightModel2, glm::vec3(0.2, 0.2, 0.2));
                }
            }
        }

        #pragma endregion

        #pragma region Drone Battery

        if (isDrone1Active && drone1BatteryPercentage > 0)
            drone1BatteryPercentage -= batteryChange;
        if (isDrone2Active && drone2BatteryPercentage > 0)
            drone2BatteryPercentage -= batteryChange;
        if (isDrone1CameraActive && drone1BatteryPercentage > 0)
            drone1BatteryPercentage -= 0.2 * batteryChange;
        if (isDrone2CameraActive && drone2BatteryPercentage > 0)
            drone2BatteryPercentage -= 0.2 * batteryChange;

        if (!isDrone1Active && drone1BatteryPercentage < 100)
            drone1BatteryPercentage += batteryChange;
        if (!isDrone2Active && drone2BatteryPercentage < 100)
            drone2BatteryPercentage += batteryChange;
        if (drone1BatteryPercentage > 100)
            drone1BatteryPercentage = 100;
        if (drone2BatteryPercentage > 100)
            drone2BatteryPercentage = 100;

        if (drone1BatteryPercentage <= 0) {
            isDrone1Destroyed = true;
            isDrone1Active = false;
            isDrone1CameraActive = false;
        }

        if (drone2BatteryPercentage <= 0) {
            isDrone2Destroyed = true;
            isDrone2Active = false;
            isDrone2CameraActive = false;
        }

        #pragma endregion

        #pragma region Drone Destruction Check

        if (!isDrone1Destroyed && drone1Y < 0.0) {
            isDrone1Destroyed = true;
            isDrone1Active = false;
            isDrone1CameraActive = false;
        }
        if (!isDrone2Destroyed && drone2Y < 0.0) {
            isDrone2Destroyed = true;
            isDrone2Active = false;
            isDrone2CameraActive = false;
        }
        if (isDrone1Active && isDrone2Active)
        {
            float droneMutualDistance = sqrt(pow(drone1X - drone2X, 2) + pow(drone1Z - drone2Z, 2));
            if (droneMutualDistance <= droneSize && abs(drone1Y - drone2Y) < 0.2) {
                isDrone1Destroyed = true;
                isDrone1Active = false;
                isDrone1CameraActive = false;
                isDrone2Destroyed = true;
                isDrone2Active = false;
                isDrone2CameraActive = false;
            }
        }

        if (isDrone1Active) {
            float drone1ForbiddenZoneDistance = sqrt(pow(drone1X - forbiddenZoneX, 2) + pow(drone1Z - forbiddenZoneY, 2));
            if (drone1ForbiddenZoneDistance <= droneSize / 4 + forbiddenZoneRadius) {
                isDrone1Destroyed = true;
                isDrone1Active = false;
                isDrone1CameraActive = false;
            }
        }

        if (isDrone2Active) {
            float drone2ForbiddenZoneDistance = sqrt(pow(drone2X - forbiddenZoneX, 2) + pow(drone2Z - forbiddenZoneY, 2));
            if (drone2ForbiddenZoneDistance <= droneSize / 4 + forbiddenZoneRadius) {
                isDrone2Destroyed = true;
                isDrone2Active = false;
                isDrone2CameraActive = false;
            }
        }

        #pragma endregion

        #pragma region Light Positions

        glm::vec3 drone1LightPositions[] = {
            glm::vec3(redLightModel1[3][0],  redLightModel1[3][1],  redLightModel1[3][2]),
            glm::vec3(greenLightModel1[3][0],  greenLightModel1[3][1],  greenLightModel1[3][2]),
            glm::vec3(whiteLightModel1[3][0],  whiteLightModel1[3][1],  whiteLightModel1[3][2]),
        };

        glm::vec3 drone2LightPositions[] = {
            glm::vec3(redLightModel2[3][0],  redLightModel2[3][1],  redLightModel2[3][2]),
            glm::vec3(greenLightModel2[3][0],  greenLightModel2[3][1],  greenLightModel2[3][2]),
            glm::vec3(whiteLightModel2[3][0],  whiteLightModel2[3][1],  whiteLightModel2[3][2]),
        };

        #pragma endregion

        // Viewports

        #pragma region Main Viewport

        glViewport(0, 0, wWidth / 2, wHeight);

        unifiedShader.use();
        unifiedShader.setMat4("uP", projection);
        unifiedShader.setMat4("uV", view);
        unifiedShader.setMat4("uM", mapModel);

        drawMapTexture(unifiedShader, mapTexture, specularMapTexture, VAO[mapTextureIndex]);

        unifiedShader.setVec3("viewPos", glm::vec3(cameraEyeX, cameraEyeY, cameraEyeZ));


        unifiedShader.setVec3("pointLights[0].position", drone1LightPositions[0][0], drone1LightPositions[0][1], drone1LightPositions[0][2]);
        unifiedShader.setVec3("pointLights[1].position", drone1LightPositions[1][0], drone1LightPositions[1][1], drone1LightPositions[1][2]);
        unifiedShader.setVec3("pointLights[2].position", drone1LightPositions[2][0], drone1LightPositions[2][1], drone1LightPositions[2][2]);
        unifiedShader.setVec3("pointLights[3].position", drone2LightPositions[0][0], drone2LightPositions[0][1], drone2LightPositions[0][2]);
        unifiedShader.setVec3("pointLights[4].position", drone2LightPositions[1][0], drone2LightPositions[1][1], drone2LightPositions[1][2]);
        unifiedShader.setVec3("pointLights[5].position", drone2LightPositions[2][0], drone2LightPositions[2][1], drone2LightPositions[2][2]);

        if (!isDrone1Destroyed) {
            unifiedShader.use();
            unifiedShader.setMat4("uM", model1);
            drone1.Draw(unifiedShader);

            unifiedShader.use();
            unifiedShader.setMat4("uP", projection);
            unifiedShader.setMat4("uV", view);
            unifiedShader.setMat4("uM", redLightModel1);
            redLight.Draw(unifiedShader);
            unifiedShader.setMat4("uM", greenLightModel1);
            greenLight.Draw(unifiedShader);
            unifiedShader.setMat4("uM", whiteLightModel1);
            whiteLight.Draw(unifiedShader);

        }

        if (!isDrone2Destroyed) {
            unifiedShader.use();
            unifiedShader.setMat4("uM", model2);
            drone2.Draw(unifiedShader);

            unifiedShader.use();
            unifiedShader.setMat4("uP", projection);
            unifiedShader.setMat4("uV", view);
            unifiedShader.setMat4("uM", redLightModel2);
            redLight.Draw(unifiedShader);
            unifiedShader.setMat4("uM", greenLightModel2);
            greenLight.Draw(unifiedShader);
            unifiedShader.setMat4("uM", whiteLightModel2);
            whiteLight.Draw(unifiedShader);
        }
        #pragma endregion
        
        #pragma region Drone 1 Camera

        glViewport(wWidth / 2, wHeight / 2, wWidth / 4, wHeight / 2);
        if (!isDrone1Destroyed && isDrone1CameraActive) {
            drone1ViewCenterX = drone1X + (1.0 * cos(drone1Angle * 0.0174533));
            drone1ViewCenterZ = drone1Z - (1.0 * sin(drone1Angle * 0.0174533));
            glm::mat4 drone1View = glm::lookAt(glm::vec3(drone1X, drone1Y, drone1Z), glm::vec3(drone1ViewCenterX, drone1Y, drone1ViewCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", drone1View);
            unifiedShader.setMat4("uP", projection);

            if (!isDrone2Destroyed) {
                unifiedShader.setMat4("uM", model2);
                drone2.Draw(unifiedShader);

                unifiedShader.setMat4("uM", redLightModel2);
                redLight.Draw(unifiedShader);

                unifiedShader.setMat4("uM", greenLightModel2);
                greenLight.Draw(unifiedShader);

                unifiedShader.setMat4("uM", whiteLightModel2);
                whiteLight.Draw(unifiedShader);
            }

            unifiedShader.setMat4("uM", mapModel);
            drawMapTexture(unifiedShader, mapTexture, specularMapTexture, VAO[mapTextureIndex]);   
        }
        else {
            drawBackgroundTexture(noSignalShader, noSignalTexture, VAO[noSignalIndex]);
        }

        #pragma endregion

        #pragma region Drone 2 Camera

        glViewport(3 * wWidth / 4, wHeight / 2, wWidth / 4, wHeight / 2);
        if (!isDrone2Destroyed && isDrone2CameraActive) {
            drone2ViewCenterX = drone2X + (1.0 * cos(drone2Angle * 0.0174533));
            drone2ViewCenterZ = drone2Z - (1.0 * sin(drone2Angle * 0.0174533));
            glm::mat4 drone2View = glm::lookAt(glm::vec3(drone2X, drone2Y, drone2Z), glm::vec3(drone2ViewCenterX, drone2Y, drone2ViewCenterZ), glm::vec3(0.0f, 1.0f, 0.0f));
            unifiedShader.use();
            unifiedShader.setMat4("uV", drone2View);
            unifiedShader.setMat4("uP", projection);

            if (!isDrone1Destroyed) {
                unifiedShader.setMat4("uM", model1);
                drone1.Draw(unifiedShader);

                unifiedShader.setMat4("uM", redLightModel1);
                redLight.Draw(unifiedShader);

                unifiedShader.setMat4("uM", greenLightModel1);
                greenLight.Draw(unifiedShader);

                unifiedShader.setMat4("uM", whiteLightModel1);
                whiteLight.Draw(unifiedShader);
            }
            
            unifiedShader.setMat4("uM", mapModel);
            drawMapTexture(unifiedShader, mapTexture, specularMapTexture, VAO[mapTextureIndex]);
        }
        else {
            drawBackgroundTexture(noSignalShader, noSignalTexture, VAO[noSignalIndex]);
        }

        #pragma endregion

        #pragma region Info

        glViewport(3 * wWidth / 4, 0, wWidth / 4, wHeight / 2);
        drawBackgroundTexture(signatureShader, signatureTexture, VAO[signatureIndex]);
        drawActiveIndicator(activeIndicatorsShader, VAO[drone1ActiveIndicatorIndex], sizeof(drone1IndicatorVertices) / indicatorStride, newColLoc, isDrone1Active, isDrone1Destroyed);
        drawActiveIndicator(activeIndicatorsShader, VAO[drone2ActiveIndicatorIndex], sizeof(drone2IndicatorVertices) / indicatorStride, newColLoc, isDrone2Active, isDrone2Destroyed);
        if (!isDrone1Destroyed)
            drawBatteryBar(batteryBarShader, VAO[batteryBar1Index], sizeof(batteryBar1Vertices) / batteryBarStride, batteryLvl, drone1BatteryPercentage);
        if (!isDrone2Destroyed)
            drawBatteryBar(batteryBarShader, VAO[batteryBar2Index], sizeof(batteryBar2Vertices) / batteryBarStride, batteryLvl, drone2BatteryPercentage);

        #pragma endregion

        #pragma region Minimap

        glViewport(wWidth / 2, 0, wWidth / 4, wHeight / 2);

        unifiedShader.use();
        unifiedShader.setBool("isMinimap", true);
        unifiedShader.setMat4("uM", mapModel);
        unifiedShader.setMat4("uP", minimapProjection);
        unifiedShader.setMat4("uV", minimapView);
        drawBackgroundTexture(unifiedShader, minimapTexture, VAO[mapTextureIndex]);

        if (!isDrone1Destroyed) {
            drawDroneMinimap(droneMinimapShader, VAO[drone1MinimapIndex], sizeof(drone1MinimapVertices) / droneMinimapStride, model1, drone1Y);
        }
        if (!isDrone2Destroyed) {
            drawDroneMinimap(droneMinimapShader, VAO[drone2MinimapIndex], sizeof(drone2MinimapVertices) / droneMinimapStride, model2, drone2Y);
        }

        unifiedShader.setBool("isMinimap", false);

        forbiddenZoneShader.use();
        forbiddenZoneShader.setMat4("uM", glm::mat4(1.0f));
        forbiddenZoneShader.setMat4("uP", minimapProjection);
        forbiddenZoneShader.setMat4("uV", minimapView);
        drawForbiddenZone(forbiddenZoneShader, VAO[forbiddenZoneIndex], sizeof(forbiddenZoneVertices) / forbiddenZoneStride);


        #pragma endregion

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        cout << "Textura nije ucitana! Putanja texture: " << filePath << endl;
        stbi_image_free(ImageData);
        return 0;
    }
}
void drawMapTexture(Shader shader, unsigned int texture, unsigned int specularTexture, unsigned int VAO)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.use();
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularTexture);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);

}
void drawBackgroundTexture(Shader shader, unsigned int texture, unsigned int VAO)
{   
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.use();
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

}
void drawActiveIndicator(Shader shader, unsigned int VAO, unsigned int count, unsigned int newColLoc, bool isActive, bool isDestroyed)
{
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.use();
    glBindVertexArray(VAO);
    if (isDestroyed)
        glUniform3f(newColLoc, 1.0, 0.0, 0.0);
    else if (isActive)
        glUniform3f(newColLoc, 0.3647, 1.0, 0.1412);
    else
        glUniform3f(newColLoc, 0.0392, 0.1569, 0.0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);

}
void drawDroneMinimap(Shader shader, unsigned int VAO, unsigned int count, glm::mat4 model, float y)
{
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glPointSize(pointSize + 4 * y);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.use();
    glBindVertexArray(VAO);
    shader.setMat4("uM", model);
    glDrawArrays(GL_POINTS, 0, 1);
    glBindVertexArray(0);
    glUseProgram(0);
    glPointSize(1.0f);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}
void drawBatteryBar(Shader shader, unsigned int VAO, unsigned int count, unsigned int barLvl, float batteryPercentage) {
    glDisable(GL_CULL_FACE);
    shader.use();
    glBindVertexArray(VAO);
    glUniform1f(barLvl, batteryPercentage);
    glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    glBindVertexArray(0);
    glUseProgram(0);
    glEnable(GL_CULL_FACE);
}
void drawForbiddenZone(Shader shader, unsigned int VAO, unsigned int count) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    shader.use();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, count);
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
}
