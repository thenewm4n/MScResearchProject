#include "Collision.h"
#include "Fluid.h"

#include <glm/gtc/type_ptr.hpp>


void simulateFluid(const Fluid& fluid, const SDF& skullSDF, GLuint densityShader, GLuint matricesShader, GLuint forcesShader, float deltaTime)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fluid.particleCloud.ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, fluid.particleCloud.clusterSSBO);

    GLuint particleWorkGroupSize = 256;
    GLuint numParticleWorkGroups = (fluid.particleCloud.numParticles + particleWorkGroupSize - 1) / particleWorkGroupSize;

    GLuint clusterWorkGroupSize = 64;
    GLuint numClusterWorkGroups = (fluid.particleCloud.numClusters + clusterWorkGroupSize - 1) / clusterWorkGroupSize;

    // Density calculations
    glUseProgram(densityShader);
    glUniform1i(glGetUniformLocation(densityShader, "numParticles"), fluid.particleCloud.numParticles);
    glUniform1f(glGetUniformLocation(densityShader, "mass"), fluid.mass);
    glUniform1f(glGetUniformLocation(densityShader, "smoothingRadius"), fluid.smoothingRadius);
    glUniform1f(glGetUniformLocation(densityShader, "restDensity"), fluid.restDensity);
    glUniform1f(glGetUniformLocation(densityShader, "sphStiffness"), fluid.sphStiffness);
    glDispatchCompute(numParticleWorkGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Shape matching matrix calculations
    glUseProgram(matricesShader);
    glUniform1i(glGetUniformLocation(matricesShader, "numClusters"), fluid.particleCloud.numClusters);
    glUniform1i(glGetUniformLocation(matricesShader, "numParticles"), fluid.particleCloud.numParticles);
    glDispatchCompute(numClusterWorkGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Force calculations
    glUseProgram(forcesShader);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, skullSDF.textureID);

    glUniform1i(glGetUniformLocation(forcesShader, "numParticles"), fluid.particleCloud.numParticles);
    glUniform1f(glGetUniformLocation(forcesShader, "mass"), fluid.mass);
    glUniform1f(glGetUniformLocation(forcesShader, "smoothingRadius"), fluid.smoothingRadius);
    glUniform1f(glGetUniformLocation(forcesShader, "viscosity"), fluid.viscosity);
    glUniform1f(glGetUniformLocation(forcesShader, "deltaTime"), deltaTime);
    glUniform1f(glGetUniformLocation(forcesShader, "currentContractionStrength"), fluid.currentContractionStrength);
    glUniform1f(glGetUniformLocation(forcesShader, "shapeMatchingStiffness"), fluid.shapeMatchingStiffness);
    glUniform1i(glGetUniformLocation(forcesShader, "skullSDFTextureUnit"), 2);
    glUniform3fv(glGetUniformLocation(forcesShader, "skullSDFBoundingBoxMin"), 1, glm::value_ptr(skullSDF.boundingBoxMin));
    glUniform3fv(glGetUniformLocation(forcesShader, "skullSDFBoundingBoxMax"), 1, glm::value_ptr(skullSDF.boundingBoxMax));
    glUniform1f(glGetUniformLocation(forcesShader, "adhesionStrength"), fluid.adhesionStrength);
    glUniform1f(glGetUniformLocation(forcesShader, "softTissueStiffness"), fluid.softTissueStiffness);

    glDispatchCompute(numParticleWorkGroups, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}