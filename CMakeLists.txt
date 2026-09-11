cmake_minimum_required(VERSION 3.20)
project(ResearchProj VERSION 1.0)

# Ensures flat bin directory with multi-config generators
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<0:>")
if (MSVC)
    set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/debug_info")
endif()

# Dependencies
find_package(OpenGL REQUIRED)
add_subdirectory(third-party/glfw-3.4 glfw)

# Executable setup
add_executable(frontalis src/main.cpp)
target_sources(frontalis PRIVATE
    "src/Camera.cpp"
    "src/Collision.cpp"
    "src/File.cpp"
    "src/Fluid.cpp"
    "src/Input.cpp"
    "src/Mesh.cpp"
    "src/Particle.cpp"
    "src/Renderer.cpp"
    "src/Shader.cpp"
    "src/Texture.cpp"
    "src/Window.cpp"
    "third-party/glad/src/glad.c"
)

set_target_properties(frontalis PROPERTIES
    DEBUG_POSTFIX "_d"
    RELWITHDEBINFO_POSTFIX "_rd"
    RELEASE_POSTFIX ""
)

target_compile_features(frontalis PRIVATE cxx_std_17)
target_compile_definitions(frontalis PRIVATE GL_SILENCE_DEPRECATION GLM_ENABLE_EXPERIMENTAL NOMINMAX)

target_include_directories(frontalis PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third-party
    ${CMAKE_SOURCE_DIR}/third-party/glad/include
    ${CMAKE_SOURCE_DIR}/third-party/eigen-5.0.0
)
target_link_libraries(frontalis PRIVATE
    glfw
	OpenGL::GL
)