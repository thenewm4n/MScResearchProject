#define TINYOBJLOADER_IMPLEMENTATION

#include "Mesh.h"
#include "Particle.h"

#include "tiny_obj_loader.h"

#include <glm/glm.hpp>

#include <algorithm>
#include <iostream>


struct ParticleDistance
{
    int particleID;
    float squaredDistance;
};

bool compareDistances(const ParticleDistance& a, const ParticleDistance& b)
{
    return a.squaredDistance < b.squaredDistance;
}


Mesh createMesh(const float* vertexData, int numVertices)
{
    Mesh mesh;
    mesh.vertexCount = numVertices;

    // Generate and bind VAO
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);
    
    // Create buffer on GPU
    #ifdef __APPLE__                                // macOS Implementation (OpenGL 4.1) - glBufferData is the standard way to upload data in 4.1
        glGenBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, numVertices * 8 * sizeof(float), vertexData, GL_STATIC_DRAW);
    #else                                           // Windows/Linux Implementation (OpenGL 4.5) - glNamedBufferStorage is a faster, immutable storage method from 4.5
        glCreateBuffers(1, &mesh.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
        glNamedBufferStorage(mesh.vbo, numVertices * 8 * sizeof(float), vertexData, NULL);            // Allocates numVertices bytes in VRAM and copies vertex data into it
    #endif

    // Define vertex attributes
    int stride = 8 * sizeof(float);

        // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    
        // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
        
        // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return mesh;
}

Mesh loadObj(const std::string& objFile)
{
    tinyobj::attrib_t attribute;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning, error;

    // Load file
    if (!tinyobj::LoadObj(&attribute, &shapes, &materials, &warning, &error, objFile.c_str()))
    {
        std::cerr << "Mesh.cpp - OBJ loading failed: " << warning << error << std::endl;
        return Mesh{ {}, 0, 0, 0 };
    }

    std::vector<float> vertices;
    std::vector<glm::vec3> vertexPositions;

    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            // Position
            float vertexPositionX = attribute.vertices[3 * index.vertex_index + 0];
            float vertexPositionY = attribute.vertices[3 * index.vertex_index + 1];
            float vertexPositionZ = attribute.vertices[3 * index.vertex_index + 2];

            vertices.push_back(vertexPositionX);
            vertices.push_back(vertexPositionY);
            vertices.push_back(vertexPositionZ);
            vertexPositions.push_back(glm::vec3(vertexPositionX, vertexPositionY, vertexPositionZ));

            // Normal
            if (index.normal_index >= 0)
            {
                vertices.push_back(attribute.normals[3 * index.normal_index + 0]);
                vertices.push_back(attribute.normals[3 * index.normal_index + 1]);
                vertices.push_back(attribute.normals[3 * index.normal_index + 2]);
            }
            else
            {
                vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);
            }

            // UV
            if (index.texcoord_index >= 0)
            {
                vertices.push_back(attribute.texcoords[2 * index.texcoord_index + 0]);
                vertices.push_back(attribute.texcoords[2 * index.texcoord_index + 1]);
            }
            else
            {
                vertices.push_back(0.0f); vertices.push_back(0.0f);
            }
        }
    }

    Mesh mesh = createMesh(vertices.data(), vertices.size() / 8);
    mesh.vertexPositions = vertexPositions;
    return mesh;
}

std::vector<ParticleMeshBinding> bindParticlesToMesh(const std::vector<glm::vec3>& meshVertices, const std::vector<Particle>& particles, float maxInfluenceDistance)
{
    uint32_t numVertices = meshVertices.size();
    uint32_t numParticles = particles.size();

    std::vector<ParticleMeshBinding> bindings(numVertices);
    
    // For each vertex...
    for (int i = 0; i < numVertices; i++)
    {
        // Calcuate squared distance to each particle
        glm::vec3 vertexPosition = meshVertices[i];
        std::vector<ParticleDistance> distances(numParticles);
        
        for (int j = 0; j < numParticles; j++)
        {
            glm::vec3 particlePosition = glm::vec3(particles[j].initialPosition);
            glm::vec3 offset = vertexPosition - particlePosition;
            distances[j].particleID = j;
            distances[j].squaredDistance = glm::dot(offset, offset);
        }

        // Sort particles distances
        int numInfluencingParticles = distances.size() > 4 ? 4 : distances.size();
        // std::sort(distances.begin(), distances.end(), compareDistances);
        std::partial_sort(distances.begin(), distances.begin() + numInfluencingParticles, distances.end(), compareDistances);
        
        // Isolate closest 4 particles
        ParticleMeshBinding binding;
        binding.particleIDs = glm::ivec4(-1);
        binding.particleWeights = glm::vec4(0.0f);

        float sumOfParticleWeights = 0.0f;
        for (int j = 0; j < numInfluencingParticles; j++)
        {
            if (distances[j].squaredDistance >= maxInfluenceDistance * maxInfluenceDistance)
            {
                continue;
            }

            binding.particleIDs[j] = distances[j].particleID;
            
            float distance = distances[j].squaredDistance < 0.00001 ? 0.00001 : distances[j].squaredDistance;
            float particleWeight = 1.0f / distance;

            binding.particleWeights[j] = particleWeight;
            sumOfParticleWeights += particleWeight;
        }

        // Scale the weights to ensure smooth transition between moving and stationary mesh vertices
        if (sumOfParticleWeights > 0.0f)
        {
            binding.particleWeights /= sumOfParticleWeights;

            // Smoothstep between the distance of the closest particle and a maximum influence distance
            float closestDistance = std::sqrt(distances[0].squaredDistance);
            float t = std::max(0.0f, std::min(1.0f, closestDistance / maxInfluenceDistance));
            float globalFalloff = 1.0f - (t * t * (3.0f - 2.0f * t));

            binding.particleWeights *= globalFalloff;
        }
        
        bindings[i] = binding;
    }

    return bindings;
}