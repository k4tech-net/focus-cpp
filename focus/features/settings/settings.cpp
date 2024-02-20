#include "settings.hpp"

std::string Settings::readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting) {
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

    if (!jsonData.is_object()) {
        std::cerr << "Invalid JSON format." << std::endl;
        return;
    }

    Settings setting;

	std::string mode = jsonData["mode"];

    if (mode == "Generic" || mode == "generic") {
       setting.charactername = "Generic";

        for (auto& item : jsonData.items()) {
            if (item.key() != "mode") {
                weaponData weapon;

                weapon.weaponname = item.key();
                auto& valueArray = item.value();

                if (valueArray.is_array() && valueArray.size() >= 3) {
                    weapon.autofire = valueArray[0].get<bool>();
                    weapon.xdeadtime = valueArray[1].get<int>();

                    // Extract nested arrays of integers from index 2 onwards
                    for (size_t i = 2; i < valueArray.size(); ++i) {
                        if (valueArray[i].is_array()) {
                            weapon.values.push_back(valueArray[i].get<std::vector<int>>());
                        }
                        else {
                            std::cerr << "Invalid nested array for weapon: " << weapon.weaponname << std::endl;
                        }
                    }

                    setting.weapondata.push_back(weapon);
                }
                else {
                    std::cerr << "Invalid format for weapon: " << weapon.weaponname << std::endl;
                }
            }
        }

        // Set all options to false
        setting.options = std::vector<bool>(setting.weapondata.size(), false);

        settings.push_back(setting);
	}
	else if (mode == "Character" || mode == "character") {
        for (auto& characterItem : jsonData.items()) {
            if (characterItem.key() != "mode") {
                setting.charactername = characterItem.key();

                auto& characterData = characterItem.value();
                if (!characterData.is_object()) {
                    std::cerr << "Invalid data for character: " << setting.charactername << std::endl;
                    continue;
                }

                for (auto& weaponItem : characterData.items()) {
                    if (weaponItem.key() == "Options") {
                        setting.options = weaponItem.value().get<std::vector<bool>>();
                    }
                    else {
                        weaponData weapon;

                        weapon.weaponname = weaponItem.key();
                        auto& valueArray = weaponItem.value();

                        if (valueArray.is_array() && valueArray.size() >= 3) {
                            weapon.autofire = valueArray[0].get<bool>();
                            weapon.xdeadtime = valueArray[1].get<int>();

                            // Extract nested arrays of integers from index 2 onwards
                            for (size_t i = 2; i < valueArray.size(); ++i) {
                                if (valueArray[i].is_array()) {
                                    weapon.values.push_back(valueArray[i].get<std::vector<int>>());
                                }
                                else {
                                    std::cerr << "Invalid nested array for weapon: " << weapon.weaponname << std::endl;
                                }
                            }

                            setting.weapondata.push_back(weapon);
                        }
                        else {
                            std::cerr << "Invalid format for weapon: " << weapon.weaponname << std::endl;
                        }
                    }
                }

                settings.push_back(setting);
            }
        }
    }
    else {
		std::cerr << "Invalid mode: " << mode << std::endl;
    }

    return mode;
}