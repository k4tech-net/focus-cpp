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
    std::string mode;
    std::string name;
    bool autofire;
    int xdeadtime;
    std::vector<std::vector<int>> values;

    // Method to check if two Settings objects are equal
    bool operator==(const Settings& other) const {
        return mode == other.mode &&
            name == other.name &&
            autofire == other.autofire &&
            xdeadtime == other.xdeadtime &&
            values == other.values;
    }

    void readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting);
    void printSettings(const std::vector<Settings>& settings);
};