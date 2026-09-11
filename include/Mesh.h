#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

struct Particle;

struct Mesh
{
    std::vector<glm::vec3> vertexPositions;
    GLuint vao = 0;
    GLuint vbo = 0;
    int vertexCount = 0;
};

struct ParticleMeshBinding
{
    glm::ivec4 particleIDs;
    glm::vec4 particleWeights;
};

Mesh createMesh(const float* vertexData, int numVertices);
Mesh loadObj(const std::string& objFile);
std::vector<ParticleMeshBinding> bindParticlesToMesh(const std::vector<glm::vec3>& meshVertices, const std::vector<Particle>& particles, float maxInfluenceDistance);