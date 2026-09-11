#include "File.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>


std::string readSourceFile(const char* sourceFilename)
{
    std::filesystem::path assetsPath;
    std::filesystem::path currentPath = std::filesystem::current_path();

    // Search for /assets path
    while (true)
    {
        if (std::filesystem::exists(currentPath / "assets"))
        {
            assetsPath = currentPath / "assets";
            break;
        }

        std::filesystem::path parent = currentPath.parent_path();
        if (parent == currentPath) break;

        currentPath = parent;
    }

    if (assetsPath.empty())
    {
        std::cerr << "File.cpp: Could not find Assets directory." << std::endl;
        return "";
    }

    // If /assets found, try to read file and copy contents to buffer
    std::string sourcePath = (assetsPath / "shaders" / sourceFilename).string();

    std::ifstream file(sourcePath);
    if (!file.is_open())
    {
        std::cerr << "File.cpp: Could not open source file: " << sourcePath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}