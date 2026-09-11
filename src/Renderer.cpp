#include "Constants.h"
#include "Particle.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>


void drawSkull(GLuint shaderProgram, const RenderObject& skullObject)
{
    // Bind skull VAO and model matrix
    glBindVertexArray(skullObject.geometry.vao);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(skullObject.modelMatrix));

    // Make skull grey
    glUniform1i(glGetUniformLocation(shaderProgram, "useTexture"), GL_FALSE);
    glUniform3fv(glGetUniformLocation(shaderProgram, "objectColour"), 1, glm::value_ptr(skullObject.colour));

    // Draw skull vertices
    glDrawArrays(GL_TRIANGLES, 0, skullObject.geometry.vertexCount);
    glBindVertexArray(0);
}

void addLights(const RenderContext& context)
{
    // Directional lights
    glUniform3fv(glGetUniformLocation(context.shaderProgram, "moonlight.direction"), 1, glm::value_ptr(context.dirLight.direction));
    glUniform3fv(glGetUniformLocation(context.shaderProgram, "moonlight.colour"), 1, glm::value_ptr(context.dirLight.colour));
    glUniform1f(glGetUniformLocation(context.shaderProgram, "moonlight.intensity"), context.dirLight.intensity);

    // Spot lights
    glUniform1i(glGetUniformLocation(context.shaderProgram, "numSpotLights"), static_cast<GLint>(context.spotLights.size()));
    for (size_t i = 0; i < context.spotLights.size(); i++)
    {
        std::string baseString = "spotLights[" + std::to_string(i) + "].";
        glUniform3fv(glGetUniformLocation(context.shaderProgram, (baseString + "position").c_str()), 1, glm::value_ptr(context.spotLights[i].position));
        glUniform3fv(glGetUniformLocation(context.shaderProgram, (baseString + "direction").c_str()), 1, glm::value_ptr(context.spotLights[i].direction));
        glUniform3fv(glGetUniformLocation(context.shaderProgram, (baseString + "colour").c_str()), 1, glm::value_ptr(context.spotLights[i].colour));
        glUniform1f(glGetUniformLocation(context.shaderProgram, (baseString + "intensity").c_str()), context.spotLights[i].intensity);
        glUniform1f(glGetUniformLocation(context.shaderProgram, (baseString + "cutoffAngle").c_str()), context.spotLights[i].cutoffAngle);
    }
}

void drawObject(const RenderContext& context, const RenderObject& object)
{
    glUniformMatrix4fv(glGetUniformLocation(context.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(object.modelMatrix));

    // If object has texture, use it, otherwise use base colour
    if (object.texture != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, object.texture);
        glUniform1i(glGetUniformLocation(context.shaderProgram, "diffuseTexture"), 0);
        glUniform1i(glGetUniformLocation(context.shaderProgram, "useTexture"), GL_TRUE);
    }
    else
    {
        glUniform1i(glGetUniformLocation(context.shaderProgram, "useTexture"), GL_FALSE);
        glUniform3fv(glGetUniformLocation(context.shaderProgram, "objectColour"), 1, glm::value_ptr(object.colour));
    }

    glBindVertexArray(object.geometry.vao);
    glDrawArrays(GL_TRIANGLES, 0, object.geometry.vertexCount);
    glBindVertexArray(0);
}

void drawParticles(const RenderContext& context, const ParticleCloud& particleCloud)
{
    glDisable(GL_DEPTH_TEST);

    glUseProgram(context.shaderProgram);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleCloud.ssbo);

    glUniformMatrix4fv(glGetUniformLocation(context.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(context.viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(context.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(context.projectionMatrix));

    glEnable(GL_PROGRAM_POINT_SIZE);        // So vertex shader can scale particle size

    glBindVertexArray(particleCloud.vao);
    glDrawArrays(GL_POINTS, 0, particleCloud.numParticles);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

// Main draw function - draws geometry with lighting
void drawFrame(const RenderContext& context, const std::vector<RenderObject>& objects)
{
    // Prepare shader and camera
    glUseProgram(context.shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(context.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(context.viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(context.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(context.projectionMatrix));
    glUniform3fv(glGetUniformLocation(context.shaderProgram, "cameraPosition"), 1, glm::value_ptr(context.cameraPosition));

    // Bind muscle particle SSBOs for mesh vertex binding in vertex shader
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, context.leftFrontalisSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, context.rightFrontalisSSBO);

    // Bind lights
    addLights(context);

    // Draw geometry
    for (const RenderObject& object : objects)
    {
        drawObject(context, object);
    }

    glBindVertexArray(0);
}