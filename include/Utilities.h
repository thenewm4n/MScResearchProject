#pragma once

#include <Eigen/Core>
#include <glm/glm.hpp>


inline glm::vec3 eigenVecToGlmVec(const Eigen::Vector3d& eigenVec)
{
    return glm::vec3(static_cast<float>(eigenVec(0)), static_cast<float>(eigenVec(1)), static_cast<float>(eigenVec(2)));
}