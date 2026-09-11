#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>


struct SDF
{
    GLuint textureID;
    glm::vec3 boundingBoxMin;
    glm::vec3 boundingBoxMax;
    glm::ivec3 gridResolution;
    std::vector<float> distances;       // Cachced for collision robustness measurement
};

GLuint createSDFTexture(const char* objFilePath, SDF& sdf);