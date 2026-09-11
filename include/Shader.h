#pragma once

#include <glad/glad.h>


GLuint compileShaderProgram(const char* vertexSourceFilename, const char* fragSourceFilename);
GLuint compileComputeShaderProgram(const char* computeSourceFilename);