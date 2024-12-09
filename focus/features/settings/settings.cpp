#include "settings.hpp"

Constants constants;
Globals globals;
Settings settings;

void Settings::readSettings(const std::string& filename, bool clearExisting, bool updateAimbotInfo) {
    if (clearExisting) {
        characters.clear();
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << xorstr_("Error opening file: ") << filename << std::endl;
        return;
    }

    std::string line;
    characterData currentCharacter;
    weaponData currentWeapon;
    int weaponCount = 0;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key, value;
        std::getline(iss, key, '=');
        std::getline(iss, value);

        if (key == xorstr_("Mode")) {
            mode = value;
        }
        else if (key == xorstr_("Potato")) {
            potato = (value == xorstr_("1"));
        }
        else if (key == xorstr_("AimAssist") && updateAimbotInfo) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); aimbotData.type = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.provider = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.maxDistance = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.hitbox = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.confidence = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.forceHitbox = item == xorstr_("1");
			std::getline(ss, item, ','); aimbotData.fov = std::stoi(item);
        }
        else if (key == xorstr_("PidSettings")) {
			std::istringstream ss(value);
			std::string item;
            std::getline(ss, item, ','); aimbotData.pidSettings.pidPreset = std::stoi(item);
			std::getline(ss, item, ','); aimbotData.pidSettings.proportional = std::stof(item);
			std::getline(ss, item, ','); aimbotData.pidSettings.integral = std::stof(item);
			std::getline(ss, item, ','); aimbotData.pidSettings.derivative = std::stof(item);
            std::getline(ss, item, ','); aimbotData.pidSettings.rampUpTime = std::stof(item);
        }
        else if (key == xorstr_("Extras")) {
			std::istringstream ss(value);
			std::string item;
            std::getline(ss, item, ','); extras.aimKeyMode = std::stoi(item);
            std::getline(ss, item, ','); extras.recoilKeyMode = std::stoi(item);
        }
        else if (key == xorstr_("WeaponKeybinds")) {
            wpn_keybinds.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                wpn_keybinds.push_back(item);
            }
        }
        else if (key == xorstr_("AuxKeybinds")) {
            aux_keybinds.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                aux_keybinds.push_back(item);
            }
        }
        else if (key == xorstr_("Character")) {
            if (!currentCharacter.charactername.empty()) {
                if (!currentWeapon.weaponname.empty()) {
                    currentCharacter.weapondata.push_back(currentWeapon);
                    currentWeapon = weaponData();
                }
                characters.push_back(currentCharacter);
                currentCharacter = characterData();
                weaponCount = 0;
            }
            currentCharacter.charactername = value;
        }
        else if (key == xorstr_("Options")) {
            currentCharacter.options.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                currentCharacter.options.push_back(item == xorstr_("1"));
            }
        }
        else if (key == xorstr_("SelectedWeapons")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); currentCharacter.selectedweapon[0] = std::stoi(item);
            std::getline(ss, item, ','); currentCharacter.selectedweapon[1] = std::stoi(item);
        }
        else if (key == xorstr_("Weapon")) {
            if (!currentWeapon.weaponname.empty()) {
                currentCharacter.weapondata.push_back(currentWeapon);
                weaponCount++;
                currentWeapon = weaponData();
            }
            currentWeapon.weaponname = value;
        }
        else if (key == xorstr_("RapidFire")) {
            currentWeapon.rapidfire = (value == xorstr_("1"));
        }
        else if (key == xorstr_("Attachments")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); currentWeapon.attachments[0] = std::stoi(item);
            std::getline(ss, item, ','); currentWeapon.attachments[1] = std::stoi(item);
            std::getline(ss, item, ','); currentWeapon.attachments[2] = std::stoi(item);
        }
        else if (key == xorstr_("Values")) {
            std::istringstream ss(value);
            std::string valueSet;
            while (std::getline(ss, valueSet, ';')) {
                std::vector<float> values;
                std::istringstream valueSS(valueSet);
                std::string item;
                while (std::getline(valueSS, item, ',')) {
                    if (!item.empty()) {
                        values.push_back(std::stof(item));
                    }
                }
                if (!values.empty()) {
                    currentWeapon.values.push_back(values);
                }
            }
        }
        else if (key == xorstr_("Game")) {
            game = value;
        }
        else if (key == xorstr_("Sensitivity")) {
            sensitivity.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) {
                    sensitivity.push_back(std::stof(item));
                }
            }
        }
        else if (key == xorstr_("CrouchKeybind")) {
            crouch_keybind = value;
        }
        else if (key == xorstr_("AspectRatio")) {
			aspect_ratio = std::stoi(value);
		}
        else if (key == xorstr_("Fov")) {
            fov = std::stoi(value);
        }
        else if (key == xorstr_("QuickPeekDelay")) {
			quickPeekDelay = std::stoi(value);
		}

        if (key.substr(0, 6) == "Hotkey") {
            int index = std::stoi(key.substr(6));
            if (index >= 0 && index < static_cast<size_t>(HotkeyIndex::COUNT)) {
                std::vector<int> hotkeyData;
                std::istringstream valuess(value);
                std::string token;

                while (std::getline(valuess, token, ',')) {
                    hotkeyData.push_back(std::stoi(token));
                }

                if (hotkeyData.size() >= 3) {
                    hotkeys.hotkeys[index].type.store(static_cast<InputType>(hotkeyData[0]));
                    hotkeys.hotkeys[index].vKey.store(hotkeyData[1]);
                    hotkeys.hotkeys[index].mode.store(static_cast<HotkeyMode>(hotkeyData[2]));
                }
            }
        }
    }

    // Add the last weapon and character
    if (!currentWeapon.weaponname.empty()) {
        currentCharacter.weapondata.push_back(currentWeapon);
        weaponCount++;
    }
    if (!currentCharacter.charactername.empty() || mode == xorstr_("Generic") || game == xorstr_("Rust") || game == xorstr_("Overwatch")) {
        // Set selectedPrimary and selectedSecondary based on defaultweapon
        characters.push_back(currentCharacter);
    }

    // Set the initial selected character and weapon
    if (!characters.empty()) {
        selectedCharacterIndex = 0;
        isPrimaryActive = true;
    }

    // Set the weaponDataChanged flag
    weaponDataChanged = true;
    pidDataChanged = true;
}

void Settings::saveSettings(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << xorstr_("Error opening file for writing: ") << filename << std::endl;
        return;
    }

    file << xorstr_("Mode=") << mode << xorstr_("\n");
    file << xorstr_("Potato=") << (potato ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
    file << xorstr_("AimAssist=") << aimbotData.type << xorstr_(",") << aimbotData.provider << xorstr_(",")
        << aimbotData.maxDistance << xorstr_(",") << aimbotData.hitbox << xorstr_(",") << aimbotData.confidence
        << xorstr_(",") << (aimbotData.forceHitbox ? xorstr_("1") : xorstr_("0")) << xorstr_(",") << aimbotData.fov << xorstr_("\n");
    file << xorstr_("PidSettings=") << aimbotData.pidSettings.pidPreset << xorstr_(",") << aimbotData.pidSettings.proportional << xorstr_(",")
        << aimbotData.pidSettings.integral << xorstr_(",") << aimbotData.pidSettings.derivative
        << xorstr_(",") << aimbotData.pidSettings.rampUpTime << xorstr_("\n");
    file << xorstr_("Extras=") << extras.aimKeyMode << xorstr_(",") << extras.recoilKeyMode << xorstr_("\n");

    // Save hotkeys in matching format
    for (size_t i = 0; i < static_cast<size_t>(HotkeyIndex::COUNT); i++) {
        const Hotkey& hotkey = hotkeys.hotkeys[i];
        file << "Hotkey" << i << "=" <<
            static_cast<int>(hotkey.type.load()) << "," <<
            hotkey.vKey.load() << "," <<
            static_cast<int>(hotkey.mode.load()) << "\n";
    }

    if (mode == xorstr_("Generic")) {
        for (const auto& character : characters) {
            for (const auto& weapon : character.weapondata) {
                file << xorstr_("Weapon=") << weapon.weaponname << xorstr_("\n");
                file << xorstr_("RapidFire=") << (weapon.rapidfire ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
                file << xorstr_("Values=");
                for (const auto& valueSet : weapon.values) {
                    for (const auto& value : valueSet) {
                        file << value << xorstr_(",");
                    }
                    file << xorstr_(";");
                }
                file << xorstr_("\n");
            }
        }
    }
    else if (mode == xorstr_("Character")) {
        file << xorstr_("WeaponKeybinds=");
        for (const auto& keybind : wpn_keybinds) {
            file << keybind << xorstr_(",");
        }
        file << xorstr_("\n");
        file << xorstr_("AuxKeybinds=");
        for (const auto& keybind : aux_keybinds) {
            file << keybind << xorstr_(",");
        }
        file << xorstr_("\n");

        for (const auto& character : characters) {
            file << xorstr_("Character=") << character.charactername << xorstr_("\n");
            file << xorstr_("Options=");
            for (const auto& option : character.options) {
                file << (option ? xorstr_("1") : xorstr_("0")) << xorstr_(",");
            }
            file << xorstr_("\n");
            file << xorstr_("SelectedWeapons=") << character.selectedweapon[0] << xorstr_(",") << character.selectedweapon[1] << xorstr_("\n");

            for (const auto& weapon : character.weapondata) {
                file << xorstr_("Weapon=") << weapon.weaponname << xorstr_("\n");
                file << xorstr_("RapidFire=") << (weapon.rapidfire ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
                file << xorstr_("Values=");
                for (const auto& valueSet : weapon.values) {
                    for (const auto& value : valueSet) {
                        file << value << xorstr_(",");
                    }
                    file << xorstr_(";");
                }
                file << xorstr_("\n");
            }
        }
    }
    else if (mode == xorstr_("Game")) {
        file << xorstr_("Game=") << game << xorstr_("\n");

        if (game == xorstr_("Siege")) {
            file << xorstr_("Sensitivity=");
            for (const auto& sens : sensitivity) {
                file << sens << xorstr_(",");
            }
            file << xorstr_("\n");

            file << xorstr_("AspectRatio=") << aspect_ratio << xorstr_("\n");
            file << xorstr_("Fov=") << fov << xorstr_("\n");
			file << xorstr_("QuickPeekDelay=") << quickPeekDelay << xorstr_("\n");

            for (const auto& character : characters) {
                file << xorstr_("Character=") << character.charactername << xorstr_("\n");
                file << xorstr_("Options=");
                for (const auto& option : character.options) {
                    file << (option ? xorstr_("1") : xorstr_("0")) << xorstr_(",");
                }
                file << xorstr_("\n");
                file << xorstr_("SelectedWeapons=") << character.selectedweapon[0] << xorstr_(",") << character.selectedweapon[1] << xorstr_("\n");

                for (const auto& weapon : character.weapondata) {
                    file << xorstr_("Weapon=") << weapon.weaponname << xorstr_("\n");
                    file << xorstr_("RapidFire=") << (weapon.rapidfire ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
                    file << xorstr_("Attachments=") << weapon.attachments[0] << xorstr_(",") << weapon.attachments[1] << xorstr_(",") << weapon.attachments[2] << xorstr_("\n");
                    file << xorstr_("Values=");
                    for (const auto& valueSet : weapon.values) {
                        for (const auto& value : valueSet) {
                            file << value << xorstr_(",");
                        }
                        file << xorstr_(";");
                    }
                    file << xorstr_("\n");
                }
            }
        }
        else if (game == xorstr_("Rust")) {
            file << xorstr_("Sensitivity=");
            for (const auto& sens : sensitivity) {
                file << sens << xorstr_(",");
            }
            file << xorstr_("\n");

            file << xorstr_("Fov=") << fov << xorstr_("\n");

            file << xorstr_("CrouchKeybind=") << crouch_keybind << xorstr_("\n");
            file << xorstr_("Options=");
            for (const auto& option : characters[0].options) {
                file << (option ? xorstr_("1") : xorstr_("0")) << xorstr_(",");
            }
            file << xorstr_("\n");

            for (const auto& weapon : characters[0].weapondata) {
                file << xorstr_("Weapon=") << weapon.weaponname << xorstr_("\n");
                file << xorstr_("RapidFire=") << (weapon.rapidfire ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
                file << xorstr_("Values=");
                for (const auto& valueSet : weapon.values) {
                    for (const auto& value : valueSet) {
                        file << value << xorstr_(",");
                    }
                    file << xorstr_(";");
                }
                file << xorstr_("\n");
            }
        }
        else if (game == xorstr_("Overwatch")) {
            file << xorstr_("Sensitivity=");
            for (const auto& sens : sensitivity) {
                file << sens << xorstr_(",");
            }
            file << xorstr_("\n");

            file << xorstr_("Fov=") << fov << xorstr_("\n");

            file << xorstr_("Options=");
            for (const auto& option : characters[0].options) {
                file << (option ? xorstr_("1") : xorstr_("0")) << xorstr_(",");
            }
            file << xorstr_("\n");

            for (const auto& weapon : characters[0].weapondata) {
                file << xorstr_("Weapon=") << weapon.weaponname << xorstr_("\n");
                file << xorstr_("RapidFire=") << (weapon.rapidfire ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
                file << xorstr_("Values=");
                for (const auto& valueSet : weapon.values) {
                    for (const auto& value : valueSet) {
                        file << value << xorstr_(",");
                    }
                    file << xorstr_(";");
                }
                file << xorstr_("\n");
            }
        }
    }
}