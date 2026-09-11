#include "Camera.h"
#include "Collision.h"
#include "Constants.h"
#include "Fluid.h"
#include "Input.h"
#include "Mesh.h"
#include "Particle.h"
#include "Renderer.h"
#include "Shader.h"
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>


Camera camera;
glm::vec3 headPosition(0.0f, 0.0f, 0.0f);

bool renderParticles = false;
float particleSpacing = 0.05f;   // Low particle resolution: 0.08, Medium: 0.05, High: 0.04

const float BASELINE_SPACING = 0.05f;
const float BASELINE_MASS = 0.125f;
const float BASELINE_SMOOTHING_RADIUS = 0.1f;

bool performPhysicsMeasurements = false;

const float timeStep = 0.01666f;                // 16.6ms i.e 60 cycles per second

// Skin binding
float maxInfluenceDistance = 1.0f;


int main()
{
    // Initialise window and camera
    GLFWwindow* window = initialiseWindow(WIDTH, HEIGHT, "Frontalis");

    // Initialise particles, fluids (with particle clouds) and skull SDF 3D texture
    uint32_t numClustersPerFluid = 0;
    float clusterSize = 4.0f * particleSpacing;

    std::vector<Particle> leftFrontalisParticles;
    std::string leftFrontalisOBJPath = "../assets/models/frontalis_left.obj";
    initialiseParticlePositions(leftFrontalisParticles, particleSpacing, clusterSize, numClustersPerFluid, leftFrontalisOBJPath);
    Fluid leftFrontalisFluid;
    leftFrontalisFluid.particleCloud = createParticleCloud(leftFrontalisParticles, numClustersPerFluid);

    std::vector<Particle> rightFrontalisParticles;
    std::string rightFrontalisOBJPath = "../assets/models/frontalis_right.obj";
    initialiseParticlePositions(rightFrontalisParticles, particleSpacing, clusterSize, numClustersPerFluid, rightFrontalisOBJPath);    
    Fluid rightFrontalisFluid;
    rightFrontalisFluid.particleCloud = createParticleCloud(rightFrontalisParticles, numClustersPerFluid);

    float spacingRatio = particleSpacing / BASELINE_SPACING;
    float scaledRadius = BASELINE_SMOOTHING_RADIUS * spacingRatio;
    float scaledMass = BASELINE_MASS * (spacingRatio * spacingRatio * spacingRatio); // Ratio cubed

    leftFrontalisFluid.smoothingRadius = scaledRadius;
    leftFrontalisFluid.mass = scaledMass;
    rightFrontalisFluid.smoothingRadius = scaledRadius;
    rightFrontalisFluid.mass = scaledMass;

    SDF skullSDF;
    skullSDF.textureID = createSDFTexture("../assets/models/frontal_bone.obj", skullSDF);


    // Loading geometry
    std::vector<RenderObject> objects;

        // Skull
    RenderObject skullObject;
    glm::mat4 skullModelMatrix = glm::mat4(1.0f);
    skullObject.geometry = loadObj("../assets/models/skull_new.obj");
    skullObject.colour = glm::vec3(0.9f, 0.9f, 0.9f);
    skullObject.modelMatrix = skullModelMatrix;
    // objects.push_back(skullObject);

        // Frontal bone
    RenderObject frontalBoneObject;
    glm::mat4 frontalBoneModelMatrix = glm::mat4(1.0f);
    frontalBoneObject.geometry = loadObj("../assets/models/frontal_bone.obj");
    frontalBoneObject.colour = glm::vec3(0.9f, 0.9f, 0.9f);
    frontalBoneObject.modelMatrix = skullModelMatrix;
    // objects.push_back(frontalBoneObject);

        // Skin
    RenderObject skinObject;
    glm::mat4 skinModelMatrix = glm::mat4(1.0f);
    skinObject.geometry = loadObj("../assets/models/skin_scaled.obj");
    skinObject.colour = glm::vec3(0.9f, 0.9f, 0.9f);
    skinObject.modelMatrix = skinModelMatrix;
    objects.push_back(skinObject);

        // Left frontalis muscle
    RenderObject leftFrontalisObject;
    glm::mat4 leftFrontalisModelMatrix = glm::mat4(1.0f);
    leftFrontalisObject.geometry = loadObj("../assets/models/frontalis_left.obj");
    leftFrontalisObject.colour = glm::vec3(0.9f, 0.9f, 0.9f);
    leftFrontalisObject.modelMatrix = leftFrontalisModelMatrix;
    // objects.push_back(leftFrontalisObject);

        // Right frontalis muscle
    RenderObject rightFrontalisObject;
    glm::mat4 rightFrontalisModelMatrix = glm::mat4(1.0f);
    rightFrontalisObject.geometry = loadObj("../assets/models/frontalis_right.obj");
    rightFrontalisObject.colour = glm::vec3(0.9f, 0.9f, 0.9f);
    rightFrontalisObject.modelMatrix = rightFrontalisModelMatrix;
    // objects.push_back(rightFrontalisObject);

    // Bind particles to skin mesh
    std::vector<Particle> allParticles = leftFrontalisParticles;
    allParticles.insert(allParticles.end(), rightFrontalisParticles.begin(), rightFrontalisParticles.end());
    std::vector<ParticleMeshBinding> skinBindings = bindParticlesToMesh(skinObject.geometry.vertexPositions, allParticles, maxInfluenceDistance);
    GLuint bindingSSBO;
    glGenBuffers(1, &bindingSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bindingSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleMeshBinding) * skinBindings.size(), skinBindings.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bindingSSBO);

    // Initialise directional light
    DirectionalLight dirLight;
    dirLight.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
    dirLight.colour = glm::vec3(1.0f, 1.0f, 1.0f);
    dirLight.intensity = 1.0f;


    // Compile shaders
    GLuint geometryShader = compileShaderProgram("standard.vert", "standard.frag");
    if (!geometryShader)
    {
        std::cerr << "main.cpp: Geometry shader could not be compiled." << std::endl;
        return 0;
    }
    glUseProgram(geometryShader);
    glUniform1i(glGetUniformLocation(geometryShader, "numParticlesLeftFrontalis"), leftFrontalisParticles.size());
    glUseProgram(0);

    GLuint particleShader = compileShaderProgram("particles.vert", "particles.frag");
    if (!particleShader)
    {
        std::cerr << "main.cpp: Particles shader could not be compiled." << std::endl;
        return 0;
    }

    GLuint densityShader = compileComputeShaderProgram("sph_density.comp");
    if (!densityShader)
    {
        std::cerr << "main.cpp: Density compute shader could not be compiled." << std::endl;
        return 0;
    }

    GLuint matricesShader = compileComputeShaderProgram("shape_matching_polar_decomp.comp");
    if (!matricesShader)
    {
        std::cerr << "main.cpp: Shape matching matrices compute shader could not be compiled." << std::endl;
        return 0;
    }

    GLuint forcesShader = compileComputeShaderProgram("sph_forces.comp");
    if (!forcesShader)
    {
        std::cerr << "main.cpp: Forces compute shader could not be compiled." << std::endl;
        return 0;
    }


    // Performance evaluation setup
    std::ostringstream stream;
    stream << particleSpacing;                      // Removes trailing zeroes
    std::string spacingString = stream.str();

    std::string csvName = "evaluation_data_";
    if (performPhysicsMeasurements)
    {
        csvName += "physics_";
    }
    else
    {
        csvName += "performance_";
    }
    csvName += spacingString + ".csv";

    std::ofstream csvFile(csvName);
    csvFile << "Time_s,TotalFrameTime_ms,ComputeTime_ms,MeanDensityError_percent,PenetrationCount,MaxPenetrationDepth\n";

    GLuint computeQuery;
    glGenQueries(1, &computeQuery);

    bool isRecording = true;
    float recordingStartTime = glfwGetTime();


    // Render loop
    float accumulatedTime = 0.0f;    
    float timeLastFrame = glfwGetTime();            // To prevent massive first deltaTime
    while (!glfwWindowShouldClose(window))
    {
        // Poll events this frame to ensure responsiveness
        glfwPollEvents();

        // Automated contraction cycle for evaluation
        float currentTime = glfwGetTime() - recordingStartTime;
        if (currentTime > 60.0f && isRecording)
        {
            csvFile.close();
            isRecording = false;
            std::cout << "Recording complete!" << std::endl;
        }

        if (isRecording)
        {
            // Toggle contraction every 5 seconds
            if (fmod(currentTime, 10.0f) > 5.0f)
            {
                leftFrontalisFluid.currentContractionStrength = leftFrontalisFluid.maxContractionStrength;
                rightFrontalisFluid.currentContractionStrength = rightFrontalisFluid.maxContractionStrength;
            }
            else
            {
                leftFrontalisFluid.currentContractionStrength = 0.0f;
                rightFrontalisFluid.currentContractionStrength = 0.0f;
            }
        }

        // Set orbit target, even if not orbit mode
        camera.orbitTarget = headPosition;

        // Process user input
        if (isWindowFocused)
        {
            camera.processInputs(keysPressed, keysPressedLastFrame, mouseDelta, scrollDelta);
            scrollDelta = 0.0;

            if (keysPressed[GLFW_KEY_P] && !keysPressedLastFrame[GLFW_KEY_P])
            {
                renderParticles = !renderParticles;
            }

            /*
            if (keysPressed[GLFW_KEY_M])
            {
                leftFrontalisFluid.currentContractionStrength = leftFrontalisFluid.maxContractionStrength;
            }
            else
            {
                leftFrontalisFluid.currentContractionStrength = 0.0f;
            }

            if (keysPressed[GLFW_KEY_N])
            {
                rightFrontalisFluid.currentContractionStrength = rightFrontalisFluid.maxContractionStrength;
            }
            else
            {
                rightFrontalisFluid.currentContractionStrength = 0.0f;
            }
            */
        }

        /*
        // Calculate delta time
        float timeCurrentFrame = glfwGetTime();
        float deltaTime = timeCurrentFrame - timeLastFrame;
        deltaTime = deltaTime > 0.0166f ? 0.0166f : deltaTime;                      // To prevent massive deltaTime, capped at 16.6ms i.e. 60fps
        timeLastFrame = timeCurrentFrame;
        */

        // Simulate fluid
        float timeCurrentFrame = glfwGetTime();
        float frameTime = timeCurrentFrame - timeLastFrame;
        timeLastFrame = timeCurrentFrame;
        if (frameTime > 0.25f)
        {
            frameTime = 0.25f;
        }

        accumulatedTime += frameTime;

        glBeginQuery(GL_TIME_ELAPSED, computeQuery);
        while (accumulatedTime >= timeStep)
        {
            simulateFluid(leftFrontalisFluid, skullSDF, densityShader, matricesShader, forcesShader, timeStep);
            simulateFluid(rightFrontalisFluid, skullSDF, densityShader, matricesShader, forcesShader, timeStep);
            accumulatedTime -= timeStep;
        }
        glEndQuery(GL_TIME_ELAPSED);


        // Setup render context
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        
        RenderContext context;
        context.shaderProgram = geometryShader;
        context.viewMatrix = camera.getViewMatrix();
        context.projectionMatrix = camera.getProjectionMatrix((float)currentWidth / (float)currentHeight);
        context.cameraPosition = camera.position;
        context.dirLight = dirLight;
        context.leftFrontalisSSBO = leftFrontalisFluid.particleCloud.ssbo;
        context.rightFrontalisSSBO = rightFrontalisFluid.particleCloud.ssbo;

        // Render pass
        glViewport(0, 0, currentWidth, currentHeight);      // Restore window viewport
        glClearBufferfv(GL_COLOR, 0, BACKGROUND);
        glClear(GL_DEPTH_BUFFER_BIT);

        drawFrame(context, objects);

        if (renderParticles)
        {
            context.shaderProgram = particleShader;
            drawParticles(context, leftFrontalisFluid.particleCloud);
            drawParticles(context, rightFrontalisFluid.particleCloud);
        }

        glfwSwapBuffers(window);

        // Extracting measurements after GPU has been allowed time to complete
        if (isRecording)
        {
            // Extract SPH compute time
            GLuint64 computeTimeNs;
            glGetQueryObjectui64v(computeQuery, GL_QUERY_RESULT, &computeTimeNs);
            float computeTimeMs = (float)computeTimeNs / 1000000.0f;

            // Extract total frame time
            float totalFrameTimeMs = frameTime * 1000.0f;

            // Exclude physics measurements (mean density error and collision robustness) from performance pass
            float meanDensityError = 0.0f;
            int numPenetratingParticles = 0;
            float maxPenetrationDepth = 0.0f;
            if (performPhysicsMeasurements)
            {
                // Bind left frontalis SSBO to calculate mean density error
                glBindBuffer(GL_SHADER_STORAGE_BUFFER, leftFrontalisFluid.particleCloud.ssbo);
                Particle* mappedParticles = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
                if (mappedParticles)
                {
                    for (uint32_t i = 0; i < leftFrontalisFluid.particleCloud.numParticles; i++)
                    {
                        float density = mappedParticles[i].currentPosition.w;
                        float error = std::abs(density - leftFrontalisFluid.restDensity) / leftFrontalisFluid.restDensity;
                        meanDensityError += error;

                        glm::vec3 particlePosition = glm::vec3(mappedParticles[i].currentPosition);
                        glm::vec3 normalisedParticlePosition = (particlePosition - skullSDF.boundingBoxMin) / (skullSDF.boundingBoxMax - skullSDF.boundingBoxMin);

                        if (normalisedParticlePosition.x >= 0.0f && normalisedParticlePosition.x <= 1.0f &&
                            normalisedParticlePosition.y >= 0.0f && normalisedParticlePosition.y <= 1.0f &&
                            normalisedParticlePosition.z >= 0.0f && normalisedParticlePosition.z <= 1.0f
                        )
                        {
                            glm::ivec3 gridPosition = glm::round(normalisedParticlePosition * glm::vec3(skullSDF.gridResolution - 1));
                            gridPosition = glm::clamp(gridPosition, glm::ivec3(0), skullSDF.gridResolution - 1);

                            int index = (gridPosition.z * skullSDF.gridResolution.x * skullSDF.gridResolution.y) + (gridPosition.y * skullSDF.gridResolution.x) + gridPosition.x;
                            float distance = skullSDF.distances[index];

                            if (distance < 0.0f)
                            {
                                numPenetratingParticles++;
                                if (distance < maxPenetrationDepth)
                                {
                                    maxPenetrationDepth = distance;
                                }
                            }
                        }
                    }
                    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

                    meanDensityError /= leftFrontalisFluid.particleCloud.numParticles;
                    meanDensityError *= 100.0f;
                }
            }


            csvFile << currentTime << "," << totalFrameTimeMs << "," << computeTimeMs << "," << meanDensityError << "," << numPenetratingParticles << "," << maxPenetrationDepth << "\n";
        }

        memcpy(keysPressedLastFrame, keysPressed, sizeof(keysPressed));
    }

    return 1;
}