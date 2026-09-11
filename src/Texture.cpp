#include "Texture.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>


GLuint setupTexture(const char* filename)
{
	stbi_set_flip_vertically_on_load(true);

	// Generate one texture handle
	GLuint texObject;
	glGenTextures(1, &texObject);										// Params: number of texture handles to generate, texture pointer array (where to store the textures)
	glBindTexture(GL_TEXTURE_2D, texObject);

	// Wrapping config (S is horizontal, T is vertical)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);		// Params: target, parameter name, value
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Filtering config
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);		// Enables mipmaps

	// Read pixels from image
	int width, height, channels;
	unsigned char* pixels = stbi_load(filename, &width, &height, &channels, 0);						// Params: filename, image width, image height, num channels, desired channels (force to output a specific no. channels) 
	if (!pixels)
	{
		std::cerr << "Failed to load texture: " << filename << " | Reason: " << stbi_failure_reason() << std::endl;
		return 0;
	}

	// Copy pixels from RAM to VRAM
	GLenum channelMode = GL_RGB;
	if (channels == 4)
	{
		channelMode = GL_RGBA;
	}
	else if (channels == 1)
	{
		channelMode = GL_RED;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, channelMode, width, height, 0, channelMode, GL_UNSIGNED_BYTE, pixels);		// Params: target, mipmap level, internal format (how OpenGL stores it), width, height, border (legacy), format (of the source), datatype, data
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(pixels);

	return texObject;
}