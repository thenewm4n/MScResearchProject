#pragma once

#include <glad/glad.h>


constexpr int WIDTH = 1920;
constexpr int HEIGHT = 1080;
constexpr float ASPECT_RATIO = static_cast<float>(WIDTH) / HEIGHT;

constexpr GLfloat BACKGROUND[] = { 200.0f / 255.0f, 200.0f / 255.0f, 255.0f / 255.0f, 1.0f };