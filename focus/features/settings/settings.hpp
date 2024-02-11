#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Settings;
struct Globals;

class Settings 
{
public:
    std::string name;
    bool autofire;
    int xdeadtime;
    std::vector<std::vector<int>> values;

    void readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting);
    void printSettings(const std::vector<Settings>& settings);
};