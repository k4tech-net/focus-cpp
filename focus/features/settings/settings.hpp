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

struct weaponData {
    std::string weaponname;
    bool autofire;
    int xdeadtime;
    std::vector<std::vector<int>> values;
};

class Settings 
{
public:
    std::string charactername;
	std::vector<weaponData> weapondata;
    std::vector<bool> options;

    // Method to check if two Settings objects are equal
    bool operator==(const Settings& other) const {
        return charactername == other.charactername &&
            weapondata == other.weapondata &&
            options == other.options;
    }

    std::string readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting);
};