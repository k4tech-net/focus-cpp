#include "settings.hpp"

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
            std::cout << "Mode: " << value << std::endl;
        }
        else if (key == xorstr_("Potato")) {
            potato = (value == "1");
            std::cout << "Potato: " << potato << std::endl;
        }
        else if (key == xorstr_("AimAssist") && updateAimbotInfo) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); aimbotData.provider = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.smoothing = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.maxDistance = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.percentDistance = std::stof(item);
            std::getline(ss, item, ','); aimbotData.hitbox = std::stoi(item);
            std::cout << "AimAssist updated" << std::endl;
        }
        else if (key == xorstr_("WeaponKeybinds")) {
            wpn_keybinds.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                wpn_keybinds.push_back(item);
            }
            std::cout << "WeaponKeybinds updated" << std::endl;
        }
        else if (key == xorstr_("AuxKeybinds")) {
            aux_keybinds.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                aux_keybinds.push_back(item);
            }
            std::cout << "AuxKeybinds updated" << std::endl;
        }
        else if (key == xorstr_("Character")) {
            if (!currentCharacter.charactername.empty()) {
                if (!currentWeapon.weaponname.empty()) {
                    currentCharacter.weapondata.push_back(currentWeapon);
                    currentWeapon = weaponData();
                }
                characters.push_back(currentCharacter);
                std::cout << "Character added: " << currentCharacter.charactername
                    << " with " << currentCharacter.weapondata.size() << " weapons" << std::endl;
                currentCharacter = characterData();
                weaponCount = 0;
            }
            currentCharacter.charactername = value;
            std::cout << "New character: " << value << std::endl;
        }
        else if (key == xorstr_("Options")) {
            currentCharacter.options.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                currentCharacter.options.push_back(item == "1");
            }
            std::cout << "Options updated for " << currentCharacter.charactername << std::endl;
        }
        else if (key == xorstr_("SelectedWeapons")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); currentCharacter.selectedweapon[0] = std::stoi(item);
            std::getline(ss, item, ','); currentCharacter.selectedweapon[1] = std::stoi(item);
            std::cout << "SelectedWeapons updated for " << currentCharacter.charactername << std::endl;
        }
        else if (key == xorstr_("Weapon")) {
            if (!currentWeapon.weaponname.empty()) {
                currentCharacter.weapondata.push_back(currentWeapon);
                weaponCount++;
                std::cout << "Weapon added: " << currentWeapon.weaponname << " for " << currentCharacter.charactername << std::endl;
                currentWeapon = weaponData();
            }
            currentWeapon.weaponname = value;
            std::cout << "New weapon: " << value << " for " << currentCharacter.charactername << std::endl;
        }
        else if (key == xorstr_("AutoFire")) {
            currentWeapon.autofire = (value == "1");
            std::cout << "AutoFire set to " << currentWeapon.autofire << " for " << currentWeapon.weaponname << std::endl;
        }
        else if (key == xorstr_("Attachments")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); currentWeapon.attachments[0] = std::stoi(item);
            std::getline(ss, item, ','); currentWeapon.attachments[1] = std::stoi(item);
            std::getline(ss, item, ','); currentWeapon.attachments[2] = std::stoi(item);
            std::cout << "Attachments updated for " << currentWeapon.weaponname << std::endl;
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
            std::cout << "Values updated for " << currentWeapon.weaponname << std::endl;
        }
        else if (key == xorstr_("Game")) {
            game = value;
            std::cout << "Game set to: " << value << std::endl;
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
            std::cout << "Sensitivity updated" << std::endl;
        }
        else if (key == xorstr_("CrouchKeybind")) {
            crouch_keybind = value;
            std::cout << "CrouchKeybind set to: " << value << std::endl;
        }
        else if (key == xorstr_("AspectRatio")) {
			aspect_ratio = std::stoi(value);
			std::cout << "AspectRatio set to: " << aspect_ratio << std::endl;
		}
    }

    // Add the last weapon and character
    if (!currentWeapon.weaponname.empty()) {
        currentCharacter.weapondata.push_back(currentWeapon);
        weaponCount++;
        std::cout << "Last weapon added: " << currentWeapon.weaponname << " for " << currentCharacter.charactername << std::endl;
    }
    if (!currentCharacter.charactername.empty() || mode == xorstr_("Generic") || game == xorstr_("Rust")) {
        // Set selectedPrimary and selectedSecondary based on defaultweapon
        characters.push_back(currentCharacter);
        std::cout << "Last character added: " << currentCharacter.charactername << " with " << weaponCount << " weapons" << std::endl;
    }

    std::cout << "Total characters loaded: " << characters.size() << std::endl;
    for (const auto& character : characters) {
        std::cout << "Character: " << character.charactername << " has " << character.weapondata.size() << " weapons" << std::endl;
        std::cout << "  Selected Weapons: " << character.selectedweapon[0] << ", " << character.selectedweapon[1] << std::endl;
    }

    // Set the initial selected character and weapon
    if (!characters.empty()) {
        selectedCharacterIndex = 0;
        isPrimaryActive = true;
    }

    // Set the weaponDataChanged flag
    weaponDataChanged = true;
}

void Settings::saveSettings(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << xorstr_("Error opening file for writing: ") << filename << std::endl;
        return;
    }

    file << xorstr_("Mode=") << mode << "\n";
    file << xorstr_("Potato=") << (potato ? "1" : "0") << "\n";
    file << xorstr_("AimAssist=") << aimbotData.provider << "," << aimbotData.smoothing << ","
        << aimbotData.maxDistance << "," << aimbotData.percentDistance << "," << aimbotData.hitbox << "\n";

    if (mode == xorstr_("Generic")) {
        for (const auto& character : characters) {
            for (const auto& weapon : character.weapondata) {
                file << xorstr_("Weapon=") << weapon.weaponname << "\n";
                file << xorstr_("AutoFire=") << (weapon.autofire ? "1" : "0") << "\n";
                file << xorstr_("Values=");
                for (const auto& valueSet : weapon.values) {
                    for (const auto& value : valueSet) {
                        file << value << ",";
                    }
                    file << ";";
                }
                file << "\n";
            }
        }
    }
    else if (mode == xorstr_("Character")) {
        file << xorstr_("WeaponKeybinds=");
        for (const auto& keybind : wpn_keybinds) {
            file << keybind << ",";
        }
        file << "\n";
        file << xorstr_("AuxKeybinds=");
        for (const auto& keybind : aux_keybinds) {
            file << keybind << ",";
        }
        file << "\n";

        for (const auto& character : characters) {
            file << xorstr_("Character=") << character.charactername << "\n";
            file << xorstr_("Options=");
            for (const auto& option : character.options) {
                file << (option ? "1" : "0") << ",";
            }
            file << "\n";
            file << xorstr_("SelectedWeapons=") << character.selectedweapon[0] << "," << character.selectedweapon[1] << "\n";

            for (const auto& weapon : character.weapondata) {
                file << xorstr_("Weapon=") << weapon.weaponname << "\n";
                file << xorstr_("AutoFire=") << (weapon.autofire ? "1" : "0") << "\n";
                file << xorstr_("Values=");
                for (const auto& valueSet : weapon.values) {
                    for (const auto& value : valueSet) {
                        file << value << ",";
                    }
                    file << ";";
                }
                file << "\n";
            }
        }
    }
    else if (mode == xorstr_("Game")) {
        file << xorstr_("Game=") << game << "\n";

        if (game == xorstr_("Siege")) {
            file << xorstr_("Sensitivity=");
            for (const auto& sens : sensitivity) {
                file << sens << ",";
            }
            file << "\n";

            file << xorstr_("AspectRatio=") << aspect_ratio << "\n";

            for (const auto& character : characters) {
                file << xorstr_("Character=") << character.charactername << "\n";
                file << xorstr_("Options=");
                for (const auto& option : character.options) {
                    file << (option ? "1" : "0") << ",";
                }
                file << "\n";
                file << xorstr_("SelectedWeapons=") << character.selectedweapon[0] << "," << character.selectedweapon[1] << "\n";

                for (const auto& weapon : character.weapondata) {
                    file << xorstr_("Weapon=") << weapon.weaponname << "\n";
                    file << xorstr_("AutoFire=") << (weapon.autofire ? "1" : "0") << "\n";
                    file << xorstr_("Attachments=") << weapon.attachments[0] << "," << weapon.attachments[1] << "," << weapon.attachments[2] << "\n";
                    file << xorstr_("Values=");
                    for (const auto& valueSet : weapon.values) {
                        for (const auto& value : valueSet) {
                            file << value << ",";
                        }
                        file << ";";
                    }
                    file << "\n";
                }
            }
        }
        else if (game == xorstr_("Rust")) {
            file << xorstr_("Sensitivity=");
            for (const auto& sens : sensitivity) {
                file << sens << ",";
            }
            file << "\n";
            file << xorstr_("CrouchKeybind=") << crouch_keybind << "\n";
            file << xorstr_("Options=");
            for (const auto& option : characters[0].options) {
                file << (option ? "1" : "0") << ",";
            }
            file << "\n";

            for (const auto& weapon : characters[0].weapondata) {
                file << xorstr_("Weapon=") << weapon.weaponname << "\n";
                file << xorstr_("AutoFire=") << (weapon.autofire ? "1" : "0") << "\n";
                file << xorstr_("Values=");
                for (const auto& valueSet : weapon.values) {
                    for (const auto& value : valueSet) {
                        file << value << ",";
                    }
                    file << ";";
                }
                file << "\n";
            }
        }
    }
}

void Settings::convertJsonToTextConfig(const std::string& jsonFilename, const std::string& textFilename) {
    // Read JSON file
    std::ifstream jsonFile(jsonFilename);
    if (!jsonFile.is_open()) {
        std::cerr << "Error opening JSON file: " << jsonFilename << std::endl;
        return;
    }
    json jsonData;
    jsonFile >> jsonData;

    // Open text file for writing
    std::ofstream textFile(textFilename);
    if (!textFile.is_open()) {
        std::cerr << "Error opening text file for writing: " << textFilename << std::endl;
        return;
    }

    // Write mode and potato
    textFile << "Mode=" << jsonData["Mode"].get<std::string>() << "\n";
    textFile << "Potato=" << (jsonData["Potato"].get<bool>() ? "1" : "0") << "\n";

    // Write aim assist info
    auto& aimAssist = jsonData["Aim Assist"];
    textFile << "AimAssist=" << aimAssist[0] << "," << aimAssist[1] << ","
        << aimAssist[2] << "," << aimAssist[3] << "," << aimAssist[4] << "\n";

    std::string mode = jsonData["Mode"].get<std::string>();

    if (mode == "Generic") {
        for (auto& [weaponName, weaponData] : jsonData.items()) {
            if (weaponName != "Mode" && weaponName != "Potato" && weaponName != "Aim Assist") {
                textFile << "Weapon=" << weaponName << "\n";
                textFile << "AutoFire=" << (weaponData[0].get<bool>() ? "1" : "0") << "\n";
                textFile << "Values=";
                for (auto& valueSet : weaponData[1]) {
                    for (auto& value : valueSet) {
                        textFile << value << ",";
                    }
                    textFile << ";";
                }
                textFile << "\n";
            }
        }
    }
    else if (mode == "Character") {
        // Write keybinds
        textFile << "WeaponKeybinds=";
        for (auto& keybind : jsonData["Weapon Keybinds"]) {
            textFile << keybind << ",";
        }
        textFile << "\n";
        textFile << "AuxKeybinds=";
        for (auto& keybind : jsonData["Aux Keybinds"]) {
            textFile << keybind << ",";
        }
        textFile << "\n";

        for (auto& [characterName, characterData] : jsonData.items()) {
            if (characterName != "Mode" && characterName != "Potato" && characterName != "Aim Assist"
                && characterName != "Weapon Keybinds" && characterName != "Aux Keybinds") {
                textFile << "Character=" << characterName << "\n";
                textFile << "Options=";
                for (auto& option : characterData["Options"]) {
                    textFile << (option.get<bool>() ? "1" : "0") << ",";
                }
                textFile << "\n";
                // Change DefaultWeapons to SelectedWeapons
                textFile << "SelectedWeapons=" << characterData["Default Weapons"][0] << "," << characterData["Default Weapons"][1] << "\n";

                for (auto& [weaponName, weaponData] : characterData.items()) {
                    if (weaponName != "Options" && weaponName != "Default Weapons") {
                        textFile << "Weapon=" << weaponName << "\n";
                        textFile << "AutoFire=" << (weaponData[0].get<bool>() ? "1" : "0") << "\n";
                        textFile << "Values=";
                        for (auto& valueSet : weaponData[1]) {
                            for (auto& value : valueSet) {
                                textFile << value << ",";
                            }
                            textFile << ";";
                        }
                        textFile << "\n";
                    }
                }
            }
        }
    }
    else if (mode == "Game") {
        std::string game = jsonData["Game"].get<std::string>();
        textFile << "Game=" << game << "\n";

        if (game == "Siege") {
            textFile << "Sensitivity=";
            for (auto& sens : jsonData["Sensitivity"]) {
                textFile << sens << ",";
            }
            textFile << "\n";

            textFile << xorstr_("AspectRatio=") << aspect_ratio << "\n";

            for (auto& [characterName, characterData] : jsonData.items()) {
                if (characterName != "Mode" && characterName != "Potato" && characterName != "Aim Assist"
                    && characterName != "Game" && characterName != "Sensitivity") {
                    textFile << "Character=" << characterName << "\n";
                    textFile << "Options=";
                    for (auto& option : characterData["Options"]) {
                        textFile << (option.get<bool>() ? "1" : "0") << ",";
                    }
                    textFile << "\n";
                    // Change DefaultWeapons to SelectedWeapons
                    textFile << "SelectedWeapons=" << characterData["Default Weapons"][0] << "," << characterData["Default Weapons"][1] << "\n";

                    for (auto& [weaponName, weaponData] : characterData.items()) {
                        if (weaponName != "Options" && weaponName != "Default Weapons") {
                            textFile << "Weapon=" << weaponName << "\n";
                            textFile << "AutoFire=" << (weaponData[0].get<bool>() ? "1" : "0") << "\n";
                            textFile << "Attachments=" << weaponData[1][0] << "," << weaponData[1][1] << "," << weaponData[1][2] << "\n";
                            textFile << "Values=";
                            for (auto& valueSet : weaponData[2]) {
                                for (auto& value : valueSet) {
                                    textFile << value << ",";
                                }
                                textFile << ";";
                            }
                            textFile << "\n";
                        }
                    }
                }
            }
        }
        else if (game == "Rust") {
            textFile << "Sensitivity=";
            for (auto& sens : jsonData["Sensitivity"]) {
                textFile << sens << ",";
            }
            textFile << "\n";
            textFile << "CrouchKeybind=" << jsonData["Crouch Keybind"].get<std::string>() << "\n";
            textFile << "Options=";
            for (auto& option : jsonData["Options"]) {
                textFile << (option.get<bool>() ? "1" : "0") << ",";
            }
            textFile << "\n";

            for (auto& [weaponName, weaponData] : jsonData.items()) {
                if (weaponName != "Mode" && weaponName != "Potato" && weaponName != "Aim Assist"
                    && weaponName != "Game" && weaponName != "Sensitivity" && weaponName != "Crouch Keybind" && weaponName != "Options") {
                    textFile << "Weapon=" << weaponName << "\n";
                    textFile << "AutoFire=" << (weaponData[0].get<bool>() ? "1" : "0") << "\n";
                    textFile << "Values=";
                    for (auto& valueSet : weaponData[1]) {
                        for (auto& value : valueSet) {
                            textFile << value << ",";
                        }
                        textFile << ";";
                    }
                    textFile << "\n";
                }
            }
        }
    }

    std::cout << "Conversion complete. Text config saved to: " << textFilename << std::endl;
}