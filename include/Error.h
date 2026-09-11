#pragma once

#ifndef __APPLE__

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>


inline void GLAPIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		std::cerr << "OpenGL Debug: " << message << std::endl;
	}
}

#endif