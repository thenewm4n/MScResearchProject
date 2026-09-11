#pragma once

#include "Collision.h"
#include "Particle.h"

#include <glad/glad.h>
#include <glm/glm.hpp>


struct Fluid
{
    // Renderer attributes
    ParticleCloud particleCloud;

    // SPH attributes
    float mass = 0.125f;
    float smoothingRadius = 0.1f;
    float restDensity = 1000.0f;
    float sphStiffness = 1.0f;  
    float viscosity = 10.0f;  

    // Shape matching attributes
    float shapeMatchingStiffness = 0.5f;        // Has been tuned; was initially 0.2 (?), then 2.0

    // Skin constraint
    float adhesionStrength = 10.0f;            // Tuned
    float softTissueStiffness = 50.0f;          // Tuned

    // Muscle-specific attributes
    float currentContractionStrength = 0.0f;
    float maxContractionStrength = 30.0f;
};

void simulateFluid(const Fluid& fluid, const SDF& skullSDF, GLuint densityShader, GLuint matricesShader, GLuint forcesShader, float deltaTime);