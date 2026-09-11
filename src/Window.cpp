#include "Constants.h"
#include "Error.h"
#include "Input.h"
#include "Window.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>


GLFWwindow* initialiseWindow(int width, int height, std::string name)
{
    if (!glfwInit())
    {
        std::cerr << "GLFW initialisation failed." << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);                                    // DebugCallback seems to work fine on Windows without this, but just in case

    GLFWwindow* window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);        // Create a 1920x1080 window
    if (!window)
    {
        std::cerr << "GLFW failed to create window." << std::endl;
        return nullptr;
    }

    glfwMakeContextCurrent(window);                                                     // Make this window the active "Context" for OpenGL commands
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetScrollCallback(window, MouseScrollCallback);
    glfwSetWindowSizeCallback(window, ResizeCallback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD initialisation failed." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }
   
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);                              // Ensures errors are printed immediately
    if (glDebugMessageCallback)
    {
        glDebugMessageCallback(DebugCallback, nullptr);                 // Registers the error callback function in Error.h
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    return window;
}