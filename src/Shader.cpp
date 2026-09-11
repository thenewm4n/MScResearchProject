#include "Shader.h"
#include "File.h"

#include <glad/glad.h>

#include <iostream>
#include <string>


GLuint compileShaderProgram(const char* vertexSourceFilename, const char* fragSourceFilename)
{
	int success;
	char infoLog[512];

	const char* versionString =
#ifdef __APPLE__
		"#version 410 core\n";
#else
		"#version 450 core\n";
#endif

	// Compile vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::string vertexSourceBody = readSourceFile(vertexSourceFilename);

	if (vertexSourceBody.empty())
	{
		std::cerr << "Shader.cpp: Could not open vertex shader source file." << std::endl;
		return 0;
	}

	const char* vertexSources[] = { versionString, vertexSourceBody.c_str() };
	glShaderSource(vertexShader, 2, vertexSources, NULL);
	glCompileShader(vertexShader);

	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cerr << "Vertex Shader Compilation Failure - " << infoLog << std::endl;
	}

	// Compile fragment shader
	unsigned int fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string fragSourceBody = readSourceFile(fragSourceFilename);

	if (fragSourceBody.empty())
	{
		std::cerr << "Shader.cpp: Could not open fragment shader source file." << std::endl;
		return 0;
	}

	const char* fragSources[] = { versionString, fragSourceBody.c_str() };
	glShaderSource(fragShader, 2, fragSources, NULL);
	glCompileShader(fragShader);

	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragShader, 512, NULL, infoLog);
		std::cerr << "Fragment Shader Compilation Failure - " << infoLog << std::endl;
	}

	// Link shaders to program
	unsigned int program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cerr << "Shader Program Link Failure - " << infoLog << std::endl;
	}

	// Delete shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragShader);

	return program;
}

GLuint compileComputeShaderProgram(const char* computeSourceFilename)
{
	int success;
	char infoLog[512];

	const char* versionString = "#version 450 core\n";				// Compute shaders are in OpenGL 4.3 onwards; therefore, not possible on MacOS

	// Compile shader
	unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
	std::string computeSourceBody = readSourceFile(computeSourceFilename);
	if (computeSourceBody.empty())
	{
		std::cerr << "Shader.cpp: Could not open compute shader source file." << std::endl;
		return 0;
	}

	const char* computeSources[] = { versionString, computeSourceBody.c_str() };
	glShaderSource(computeShader, 2, computeSources, NULL);
	glCompileShader(computeShader);

	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(computeShader, 512, NULL, infoLog);
		std::cerr << "Compute Shader Compilation Failure - " << infoLog << std::endl;
	}

	// Link shader to program
	unsigned int program = glCreateProgram();
	glAttachShader(program, computeShader);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, 512, NULL, infoLog);
		std::cerr << "Compute Shader Program Link Failure - " << infoLog << std::endl;
	}

	// Delete shader after linking
	glDeleteShader(computeShader);

	return program;
}