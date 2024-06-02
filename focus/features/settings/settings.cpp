#include "settings.hpp"

Globals g;

void Settings::readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting) {
    if (clearExisting) {
        settings.clear();
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << xorstr_("Error opening file: ") << filename << std::endl;
        return;
    }

    json jsonData;
    file >> jsonData;

    if (!jsonData.is_object()) {
        std::cerr << xorstr_("Invalid JSON format.") << std::endl;
        return;
    }

    Settings setting;

	CHI.mode = jsonData[xorstr_("Mode")];
    CHI.potato = jsonData[xorstr_("Potato")];

    g.aimbotinfo.provider = jsonData[xorstr_("Aim Assist")][0].get<int>();
    g.aimbotinfo.smoothing = jsonData[xorstr_("Aim Assist")][1].get<int>();
    g.aimbotinfo.maxDistance = jsonData[xorstr_("Aim Assist")][2].get<int>();

    if (CHI.mode == xorstr_("Generic")) {
       setting.charactername = xorstr_("Generic");

       CHI.wpn_keybinds = { "" };
       CHI.aux_keybinds = { "" };

        for (auto& item : jsonData.items()) {
            if (item.key() != xorstr_("Mode") && item.key() != xorstr_("Potato") && item.key() != xorstr_("Aim Assist")) {
                weaponData weapon;

                weapon.weaponname = item.key();
                auto& valueArray = item.value();

                if (valueArray.is_array() && valueArray.size() >= 2) {
                    weapon.autofire = valueArray[0].get<bool>();
                    weapon.attachments = { 0, 0, 0 };

                    // Assuming the nested arrays are now wrapped in another array at index 2
                    if (valueArray[1].is_array()) {
                        auto& nestedArrays = valueArray[1];
                        for (auto& nestedArray : nestedArrays) {
                            if (nestedArray.is_array()) {
                                weapon.values.push_back(nestedArray.get<std::vector<float>>());
                            }
                            else {
                                std::cerr << xorstr_("Invalid nested array format for weapon: ") << weapon.weaponname << std::endl;
                            }
                        }
                    }
                    else {
                        std::cerr << xorstr_("Expected a nested array for weapon values: ") << weapon.weaponname << std::endl;
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
    else if (CHI.mode == xorstr_("Character")) {

        CHI.wpn_keybinds = jsonData[xorstr_("Weapon Keybinds")];
        CHI.aux_keybinds = jsonData[xorstr_("Aux Keybinds")];

        for (auto& characterItem : jsonData.items()) {
            if (characterItem.key() != xorstr_("Mode") && characterItem.key() != xorstr_("Potato") && characterItem.key() != xorstr_("Aim Assist") && characterItem.key() != xorstr_("Weapon Keybinds") && characterItem.key() != xorstr_("Aux Keybinds")) {
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

                        if (valueArray.is_array() && valueArray.size() >= 2) {
                            weapon.autofire = valueArray[0].get<bool>();
                            weapon.attachments = { 0, 0, 0 };

                            // Assuming the nested arrays are now wrapped in another array at index 2
                            if (valueArray[1].is_array()) {
                                auto& nestedArrays = valueArray[1];
                                for (auto& nestedArray : nestedArrays) {
                                    if (nestedArray.is_array()) {
                                        weapon.values.push_back(nestedArray.get<std::vector<float>>());
                                    }
                                    else {
                                        std::cerr << xorstr_("Invalid nested array format for weapon: ") << weapon.weaponname << std::endl;
                                    }
                                }
                            }
                            else {
                                std::cerr << xorstr_("Expected a nested array for weapon values: ") << weapon.weaponname << std::endl;
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
    else if (CHI.mode == xorstr_("Game")) {

        CHI.game = jsonData[xorstr_("Game")];

        if (CHI.game == xorstr_("Siege")) {
            CHI.wpn_keybinds = { "" };
            CHI.aux_keybinds = { "" };

            CHI.sensitivity = jsonData[xorstr_("Sensitivity")].get<std::vector<float>>();

            for (auto& characterItem : jsonData.items()) {
                if (characterItem.key() != xorstr_("Mode") && characterItem.key() != xorstr_("Potato") && characterItem.key() != xorstr_("Aim Assist") && characterItem.key() != xorstr_("Game") && characterItem.key() != xorstr_("Sensitivity")) {
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
                                weapon.attachments = valueArray[1].get<std::vector<int>>();

                                // Assuming the nested arrays are now wrapped in another array at index 3
                                if (valueArray[2].is_array()) {
                                    auto& nestedArrays = valueArray[2];
                                    for (auto& nestedArray : nestedArrays) {
                                        if (nestedArray.is_array()) {
                                            weapon.values.push_back(nestedArray.get<std::vector<float>>());
                                        }
                                        else {
                                            std::cerr << xorstr_("Invalid nested array format for weapon: ") << weapon.weaponname << std::endl;
                                        }
                                    }
                                }
                                else {
                                    std::cerr << xorstr_("Expected a nested array for weapon values: ") << weapon.weaponname << std::endl;
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
        else if (CHI.game == xorstr_("Rust")) {
            setting.charactername = xorstr_("Rust");

            CHI.wpn_keybinds = { "" };
            CHI.aux_keybinds = { "" };

            CHI.sensitivity = jsonData[xorstr_("Sensitivity")].get<std::vector<float>>();

            for (auto& item : jsonData.items()) {
                if (item.key() != xorstr_("Mode") && item.key() != xorstr_("Potato") && item.key() != xorstr_("Aim Assist") && item.key() != xorstr_("Game") && item.key() != xorstr_("Sensitivity") && item.key() != xorstr_("Options")) {
                    weaponData weapon;

                    weapon.weaponname = item.key();
                    auto& valueArray = item.value();

                    if (valueArray.is_array() && valueArray.size() >= 2) {
                        weapon.autofire = valueArray[0].get<bool>();
                        weapon.attachments = { 0, 0, 0 };

                        // Assuming the nested arrays are now wrapped in another array at index 2
                        if (valueArray[1].is_array()) {
                            auto& nestedArrays = valueArray[1];
                            for (auto& nestedArray : nestedArrays) {
                                if (nestedArray.is_array()) {
                                    weapon.values.push_back(nestedArray.get<std::vector<float>>());
                                }
                                else {
                                    std::cerr << xorstr_("Invalid nested array format for weapon: ") << weapon.weaponname << std::endl;
                                }
                            }
                        }
                        else {
                            std::cerr << xorstr_("Expected a nested array for weapon values: ") << weapon.weaponname << std::endl;
                        }

                        setting.weapondata.push_back(weapon);
                    }
                    else {
                        std::cerr << xorstr_("Invalid format for weapon: ") << weapon.weaponname << std::endl;
                    }
                }
            }

            // Set all options to false
            setting.options = jsonData[xorstr_("Options")].get<std::vector<bool>>();

            // Set default weapon to 0 for generic mode
            setting.defaultweapon = std::vector<int>{ 0, 0 };

            settings.push_back(setting);
		}
        else {
			std::cerr << xorstr_("Invalid game: ") << CHI.game << std::endl;
        }
    }
    else {
		std::cerr << xorstr_("Invalid mode: ") << CHI.mode << std::endl;
    }

    return;
}