#include "Collision.h"
#include "Utilities.h"

#include <Eigen/Core>
#include <igl/readOBJ.h>
#include <igl/signed_distance.h>

#include <iostream>
#include <vector>


GLuint createSDFTexture(const char* objFilePath, SDF& sdf)
{
    Eigen::MatrixXd vertexPositions;
    Eigen::MatrixXi faceVertexIndices;

    bool success = igl::readOBJ(objFilePath, vertexPositions, faceVertexIndices);
    if (!success)
    {
        std::cout << "Collision.cpp: OBJ failed to be read." << std::endl;
    }

    Eigen::Vector3d minimums = vertexPositions.colwise().minCoeff();
    Eigen::Vector3d maximums = vertexPositions.colwise().maxCoeff();

    double padding = 0.05;
    minimums -= Eigen::Vector3d::Constant(padding);
    maximums += Eigen::Vector3d::Constant(padding);

    sdf.boundingBoxMin = eigenVecToGlmVec(minimums);
    sdf.boundingBoxMax = eigenVecToGlmVec(maximums);

    sdf.gridResolution = glm::ivec3(64, 64, 64);

    // Instantiate grid point positions
    std::vector<Eigen::RowVector3d> gridPointPositions;
    gridPointPositions.reserve(sdf.gridResolution.x * sdf.gridResolution.y * sdf.gridResolution.z);
    
    for (int z = 0; z < sdf.gridResolution.z; z++)
    {
        for (int y = 0; y < sdf.gridResolution.y; y++)
        {
            for (int x = 0; x < sdf.gridResolution.x; x++)
            {
                glm::vec3 minimumsVec = eigenVecToGlmVec(minimums);
                glm::vec3 maximumsVec = eigenVecToGlmVec(maximums);
                glm::vec3 point = minimumsVec + (maximumsVec - minimumsVec) * (glm::vec3(x, y, z) / glm::vec3(sdf.gridResolution - glm::ivec3(1)));
                gridPointPositions.push_back(Eigen::RowVector3d(point.x, point.y, point.z));
            }
        }
    }

    Eigen::MatrixXd gridPositionsMatrix(gridPointPositions.size(), 3);
    for (size_t i = 0; i < gridPointPositions.size(); i++)
    {
        gridPositionsMatrix.row(i) = gridPointPositions[i];
    }

    // Calculate distances at each grid point
    Eigen::VectorXd signedDistances;
    Eigen::VectorXi closestTriangleIndices;
    Eigen::MatrixXd closestMeshPointPosition;
    Eigen::MatrixXd closestMeshPointNormal;

    igl::signed_distance(gridPositionsMatrix, vertexPositions, faceVertexIndices, igl::SIGNED_DISTANCE_TYPE_WINDING_NUMBER, signedDistances, closestTriangleIndices, closestMeshPointPosition, closestMeshPointNormal);

    std::vector<float> textureData(signedDistances.size());
    for (int i = 0; i < signedDistances.size(); i++)
    {
        textureData[i] = static_cast<float>(signedDistances(i));
    }

    // Upload to OpenGL
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, sdf.gridResolution.x, sdf.gridResolution.y, sdf.gridResolution.z, 0, GL_RED, GL_FLOAT, textureData.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_3D, 0);

    sdf.distances = textureData;

    return textureID;
}