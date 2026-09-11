#include "Particle.h"
#include "Utilities.h"

#include <Eigen/Core>
#include <igl/readOBJ.h>
#include <igl/signed_distance.h>

#include <iostream>


void initialiseParticlePositions(std::vector<Particle>& particles, float spacing, float clusterSize, uint32_t& numClustersRef, const std::string& objFilePath)
{
    Eigen::MatrixXd vertexPositions;
    Eigen::MatrixXi faceVertexIndices;          // Holds indices referring to vertexPositions

    igl::readOBJ(objFilePath, vertexPositions, faceVertexIndices);

    std::vector<Eigen::RowVector3d> particlePositionsVector;

    // Generate particles according to bounding box
    Eigen::Vector3d minimums = vertexPositions.colwise().minCoeff();        // Looks down columns to find min for x, y and z
    Eigen::Vector3d maximums = vertexPositions.colwise().maxCoeff();        // Looks down columns to find max for x, y and z

    minimums -= Eigen::Vector3d::Constant(spacing);                                   // Ensures all particles are captured
    maximums += Eigen::Vector3d::Constant(spacing);                                   // Ensures all particles are captured

    for (double i = minimums.x(); i <= maximums.x(); i += spacing)
    {
        for (double j = minimums.y(); j <= maximums.y(); j += spacing)
        {
            for (double k = minimums.z(); k <= maximums.z(); k += spacing)
            {
                particlePositionsVector.push_back(Eigen::RowVector3d(i, j, k));
            }
        }
    }

    Eigen::MatrixXd particlePositions;
    particlePositions.resize(particlePositionsVector.size(), 3);
    for (size_t i = 0; i < particlePositionsVector.size(); i++)
    {
        particlePositions.row(i) = particlePositionsVector[i];
    }

    Eigen::VectorXd particleSignedDistances;
    
    // Irrelevant for now
    Eigen::VectorXi closestTriangleIndices;             // Indices of closest triangle to each particle (i.e. rows of faceVertexIndices)
    Eigen::MatrixXd closestMeshPointPosition;
    Eigen::MatrixXd closestMeshPointNormal;

    igl::signed_distance(particlePositions, vertexPositions, faceVertexIndices, igl::SIGNED_DISTANCE_TYPE_WINDING_NUMBER, particleSignedDistances, closestTriangleIndices, closestMeshPointPosition, closestMeshPointNormal);

    for (int i = 0; i < particlePositions.rows(); i++)
    {
        if (particleSignedDistances(i) < 0.0)
        {
            Eigen::RowVector3d position = particlePositions.row(i);

            Particle particle;
            particle.currentPosition = glm::vec4(position.x(), position.y(), position.z(), 1.0f);
            particle.initialPosition = glm::vec4(position.x(), position.y(), position.z(), 0.0f);
            particle.velocity = glm::vec4(0.0f);
            particle.clusterIDsA = glm::ivec4(-1);
            particle.clusterIDsB = glm::ivec4(-1);
            
            /*
            // Fixed particle y cut-off hack
            if (particle.currentPosition.y > 3.5)
            {
                particle.initialPosition.w = 1.0f;
            }
            */

            particles.push_back(particle);
        }
    }

    // Isolate top particles and make them fixed
    for (Particle& particle : particles)
    {
        bool isTopParticle = true;
        for (const Particle& otherParticle : particles)
        {
            bool isSameColumnX = std::abs(particle.initialPosition.x - otherParticle.initialPosition.x) < 0.0001;
            bool otherIsHigher = otherParticle.initialPosition.y > particle.initialPosition.y;

            if (isSameColumnX && otherIsHigher)
            {
                isTopParticle = false;
                break;
            }
        }

        if (isTopParticle)
        {
            particle.initialPosition.w = 1.0f;
        }
    }

    // Assigning clusters for shape matching
    float clusterSpacing = spacing * 4.0f;
    glm::vec3 boundingBoxSize = eigenVecToGlmVec(maximums) - eigenVecToGlmVec(minimums);
    glm::ivec3 gridDimensions = glm::ivec3(glm::ceil(boundingBoxSize / clusterSize)) + glm::ivec3(2);      // Ceiling and add 2 to ensure all particles are inckluded
    numClustersRef = gridDimensions.x * gridDimensions.y * gridDimensions.z;

    for (Particle& particle : particles)
    {
        glm::vec3 normalisedGridPosition = (glm::vec3(particle.initialPosition) - eigenVecToGlmVec(minimums)) / clusterSize;
        glm::ivec3 leftBottomBackGridCell = glm::ivec3(glm::floor(normalisedGridPosition - glm::vec3(0.5f)));

        int clusterCount = 0;
        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                for (int k = 0; k < 2; k++)
                {
                    int clusterId = -1;
                    glm::ivec3 currentGridCell = leftBottomBackGridCell + glm::ivec3(i, j, k);

                    bool isInGrid = currentGridCell.x >= 0 && currentGridCell.x < gridDimensions.x && currentGridCell.y >= 0 && currentGridCell.y < gridDimensions.y && currentGridCell.z >= 0 && currentGridCell.z < gridDimensions.z;
                    if (isInGrid)
                    {
                        clusterId = (currentGridCell.z * gridDimensions.x * gridDimensions.y) + (currentGridCell.y * gridDimensions.x) + currentGridCell.x;
                    }

                    if (clusterCount < 4)
                    {
                        particle.clusterIDsA[clusterCount] = clusterId;
                    }
                    else if (clusterCount < 8)
                    {
                        particle.clusterIDsB[clusterCount - 4] = clusterId;
                    }

                    clusterCount++;
                }
            }
        }
    }
}

ParticleCloud createParticleCloud(const std::vector<Particle>& particles, uint32_t numClusters)
{
    ParticleCloud cloud;
    cloud.numParticles = particles.size();
    cloud.numClusters = numClusters;
    
    GLsizeiptr particleBufferSize = particles.size() * sizeof(Particle);
    glCreateBuffers(1, &cloud.ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cloud.ssbo);
    glNamedBufferStorage(cloud.ssbo, particleBufferSize, particles.data(), GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);            // GL_DYNAMIC_STORAGE_BIT required so compute shader can write to it
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, cloud.ssbo);

    GLsizeiptr clusterBufferSize = numClusters * sizeof(Cluster);
    glCreateBuffers(1, &cloud.clusterSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, cloud.clusterSSBO);
    glNamedBufferStorage(cloud.clusterSSBO, clusterBufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, cloud.clusterSSBO);

    glGenVertexArrays(1, &cloud.vao);

    return cloud;
}