#include "settings.hpp"

std::tuple<std::string, std::vector<std::string>, std::vector<std::string>> Settings::readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting) {
    if (clearExisting) {
        settings.clear();
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << xorstr_("Error opening file: ") << filename << std::endl;
        return {};
    }

    json jsonData;
    file >> jsonData;

    if (!jsonData.is_object()) {
        std::cerr << xorstr_("Invalid JSON format.") << std::endl;
        return {};
    }

    Settings setting;

	std::string mode = jsonData[xorstr_("Mode")];

	std::vector<std::string> wpn_keybinds;
    std::vector<std::string> aux_keybinds;

    if (mode == xorstr_("Generic") || mode == xorstr_("generic")) {
       setting.charactername = xorstr_("Generic");

       wpn_keybinds = { "" };
       aux_keybinds = { "" };

        for (auto& item : jsonData.items()) {
            if (item.key() != xorstr_("Mode") && item.key() != xorstr_("Weapon Keybinds") && item.key() != xorstr_("Aux Keybinds")) {
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
                            std::cerr << xorstr_("Invalid nested array for weapon: ") << weapon.weaponname << std::endl;
                        }
                    }

                    setting.weapondata.push_back(weapon);
                }
                else {
                    std::cerr << xorstr_("Invalid format for weapon: ") << weapon.weaponname << std::endl;
                }
            }
        }

        // Set all options to false
        setting.options = std::vector<bool>{ false, false, false, false, false };

        // Set default weapon to 0 for generic mode
        setting.defaultweapon = std::vector<int>{ 0, 0 };

        settings.push_back(setting);
	}
    else if (mode == xorstr_("Character") || mode == xorstr_("character")) {

        wpn_keybinds = jsonData[xorstr_("Weapon Keybinds")];
        aux_keybinds = jsonData[xorstr_("Aux Keybinds")];

        for (auto& characterItem : jsonData.items()) {
            if (characterItem.key() != xorstr_("Mode") && characterItem.key() != xorstr_("Weapon Keybinds") && characterItem.key() != xorstr_("Aux Keybinds")) {
                Settings setting; // Create a new Settings object for each character

                setting.charactername = characterItem.key();

                auto& characterData = characterItem.value();
                if (!characterData.is_object()) {
                    std::cerr << xorstr_("Invalid data for character: ") << setting.charactername << std::endl;
                    continue;
                }

                for (auto& weaponItem : characterData.items()) {
                    if (weaponItem.key() == xorstr_("Options")) {
                        setting.options = weaponItem.value().get<std::vector<bool>>();
                    }
                    else if (weaponItem.key() == xorstr_("Default Weapons")) {
                        setting.defaultweapon = weaponItem.value().get<std::vector<int>>();
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
                                    std::cerr << xorstr_("Invalid nested array for weapon: ") << weapon.weaponname << std::endl;
                                }
                            }

                            setting.weapondata.push_back(weapon);
                        }
                        else {
                            std::cerr << xorstr_("Invalid format for weapon: ") << weapon.weaponname << std::endl;
                        }
                    }
                }

                settings.push_back(setting); // Push the Settings object for the current character
            }
        }
    }
    else {
		std::cerr << xorstr_("Invalid mode: ") << mode << std::endl;
    }

    return std::make_tuple(mode, wpn_keybinds, aux_keybinds);
}