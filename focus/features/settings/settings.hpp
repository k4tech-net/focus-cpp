#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Settings;
struct Globals;

class Settings 
{
public:
    std::string name;
    std::vector<std::vector<int>> values;

    void readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting);
    void printSettings(const std::vector<Settings>& settings);
    std::optional<Settings> findSettingsByName(const std::vector<Settings>& settings, const std::string& name);
};