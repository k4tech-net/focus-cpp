#include "settings.hpp"

void Settings::readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting) {
    if (clearExisting) {
        settings.clear();
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    json jsonData;
    file >> jsonData;

    for (auto& item : jsonData.items()) {
        Settings setting;
        setting.name = item.key();
        setting.values = item.value().get<std::vector<std::vector<int>>>();
        settings.push_back(setting);
    }
}

void Settings::printSettings(const std::vector<Settings>& settings) {
    for (const auto& setting : settings) {
        std::cout << "Name: " << setting.name << std::endl;
        std::cout << "Values:" << std::endl;
        for (const auto& nestedArray : setting.values) {
            std::cout << "  [ ";
            for (const auto& value : nestedArray) {
                std::cout << value << " ";
            }
            std::cout << "]" << std::endl;
        }
        std::cout << std::endl;
    }
}

std::optional<Settings> Settings::findSettingsByName(const std::vector<Settings>& settings, const std::string& name) {
    auto it = std::find_if(settings.begin(), settings.end(), [name](const Settings& setting) {
        return setting.name == name;
        });

    if (it != settings.end()) {
        return *it; // Return the found setting
    }
    else {
        return std::nullopt; // Return std::nullopt if not found
    }
}