#pragma once

#include <glad/glad.h>


struct ShadowMap
{
    unsigned int fboID;
    unsigned int textureID;
};

inline ShadowMap setupShadowMap(int width, int height)
{
	// Create and bind the frame buffer
    ShadowMap shadow;
    glGenFramebuffers(1, &shadow.fboID);              // Generates 1 framebuffer on GPU and stores its ID in shadow.fbo
    glBindFramebuffer(GL_FRAMEBUFFER, shadow.fboID);  // Makes the shadow framebuffer the active framebuffer

	// Create, bind and reserve VRAM for the shadow map texture
    glGenTextures(1, &shadow.textureID);              // Generates 1 texture on GPU and stores its ID in shadow.texture
    glBindTexture(GL_TEXTURE_2D, shadow.textureID);   // Makes the shadow texture the active texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);   // Allocates memory for the texture

	// Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    float borderColour[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);

	// Attach the shadow map texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.textureID, 0);

	// Tell OpenGL not to write or read to colour buffers to avoid unnecessary computation
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// Unbind the frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return shadow;
}