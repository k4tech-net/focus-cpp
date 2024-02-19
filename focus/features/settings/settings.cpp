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

    Settings setting;

	setting.mode = jsonData["mode"];

    if (setting.mode == "Generic" || setting.mode == "generic") {

        for (auto& item : jsonData.items()) {
            if (item.key() != "mode") {  // Skip mode setting itself
                setting.name = item.key();

                auto& valueArray = item.value();
                if (valueArray.is_array() && valueArray.size() >= 3) {
                    setting.autofire = valueArray[0].get<bool>();
                    setting.xdeadtime = valueArray[1].get<int>();

                    // Extract nested arrays of integers from index 2 onwards
                    for (size_t i = 2; i < valueArray.size(); ++i) {
                        if (valueArray[i].is_array()) {
                            setting.values.push_back(valueArray[i].get<std::vector<int>>());
                        }
                        else {
                            std::cerr << "Invalid nested array for setting: " << setting.name << std::endl;
                        }
                    }

                    settings.push_back(setting);
                }
                else {
                    std::cerr << "Invalid format for setting: " << setting.name << std::endl;
                }
            }
        }
	}
	else if (setting.mode == "Character" || setting.mode == "character") {
        // This is the big one
    }
    else {
		std::cerr << "Invalid mode: " << setting.mode << std::endl;
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