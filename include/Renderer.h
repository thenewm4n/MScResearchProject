#pragma once

#include "Collision.h"
#include "Mesh.h"
#include "Particle.h"

#include <glm/glm.hpp>


struct DirectionalLight
{
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 colour = glm::vec3(1.0f);
    float intensity = 1.0f;
};

struct SpotLight
{
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, -1.0f, 0.0f);
    glm::vec3 colour = glm::vec3(1.0f);
    float intensity = 1.0f;
    float cutoffAngle = glm::cos(glm::radians(15.0f));
};

struct RenderContext
{
    GLuint shaderProgram = 0;
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::vec3 cameraPosition = glm::vec3(0.0f);

    DirectionalLight dirLight;
    glm::mat4 dirLightViewMatrix = glm::mat4(1.0f);
    glm::mat4 dirLightProjectionMatrix = glm::mat4(1.0f);

    std::vector<SpotLight> spotLights;

    GLuint leftFrontalisSSBO;
    GLuint rightFrontalisSSBO;
};

struct RenderObject
{
    Mesh geometry;
    GLuint texture = 0;
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 colour = glm::vec3(1.0f);
};

void drawObject(const RenderContext& context, const RenderObject& object);
void drawParticles(const RenderContext& context, const ParticleCloud& particleCloud);
void drawFrame(const RenderContext& context, const std::vector<RenderObject>& objects);
void drawShadowPass(const RenderContext& context, const std::vector<RenderObject>& objects);
void drawSDFDebug(const RenderContext& context, const SDF& sdf, GLuint debugShaderProgram);