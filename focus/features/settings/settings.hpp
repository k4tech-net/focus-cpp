#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>
#include <xorstr.hpp>
#include <variant>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Settings;
struct Globals;

struct weaponData {
    std::string weaponname;
    bool autofire;
    int xdeadtime;
    std::vector<std::vector<int>> values;

    bool operator==(const weaponData& other) const {
        return weaponname == other.weaponname &&
            autofire == other.autofire &&
            xdeadtime == other.xdeadtime &&
            values == other.values;
    }
};

class Settings 
{
public:
    std::string charactername;
	std::vector<weaponData> weapondata;
    std::vector<bool> options;
    std::vector<int> defaultweapon;

    //// Method to check if two Settings objects are equal
    //bool operator==(const Settings& other) const {
    //    return charactername == other.charactername &&
    //        weapondata == other.weapondata &&
    //        options == other.options;
    //}

    std::tuple<std::string, std::vector<std::string>, std::vector<std::string>> readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting);
};