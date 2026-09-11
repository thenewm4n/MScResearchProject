#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>


// Using packed vec4s rather than vec3s since GPU reads data in chunks, typically 16 bytes (i.e. one vec4)
struct Particle
{
	glm::vec4 initialPosition;		// w = fixed flag (i.e. 1 = fixed, 0 = fluid)
	glm::vec4 currentPosition;		// w = density
	glm::vec4 velocity;				// w = pressure
	glm::ivec4 clusterIDsA;			// -1 means no cluster
	glm::ivec4 clusterIDsB;
};

struct Cluster
{
	glm::vec4 initialCentreOfMass;
	glm::vec4 currentCentreOfMass;
	glm::mat4 rotationMatrix;
	uint32_t numParticles;
	glm::vec3 padding;						
};

struct ParticleCloud
{
	GLuint vao = 0;
	GLuint ssbo = 0;
	GLuint clusterSSBO = 0;
	uint32_t numParticles = 0;
	uint32_t numClusters = 0;
};

void initialiseParticlePositions(std::vector<Particle>& particles, float spacing, float clusterSize, uint32_t& numClustersRef, const std::string& objFilePath);
ParticleCloud createParticleCloud(const std::vector<Particle>& particles, uint32_t numClusters);