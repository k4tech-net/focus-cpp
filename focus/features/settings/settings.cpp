#include "settings.hpp"

Constants constants;
Globals globals;
Settings settings;

// New function to check and convert legacy configs
bool Settings::isLegacyConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // Check multiple markers that indicate a legacy config
    std::string line;
    while (std::getline(file, line)) {
        // Check for specific legacy markers
        if (line.find(xorstr_("Mode=")) == 0 ||
            line.find(xorstr_("Game=")) == 0) {
            return true;
        }
    }
    return false;
}


std::vector<std::string> Settings::convertAllLegacyConfigs() {
    std::vector<std::string> convertedFiles;

    for (const auto& filename : globals.filesystem.configFiles) {
        if (isLegacyConfig(filename)) {
            // Create a temporary settings object for conversion
            Settings tempSettings;

            // Initialize with default values for all settings
            // Global settings defaults
            tempSettings.globalSettings.potato = true;
            tempSettings.globalSettings.sensitivityCalculator = 0; // Generic by default
            tempSettings.globalSettings.characterDetectors.resize(1, false);
            tempSettings.globalSettings.weaponDetectors.resize(2, false);
            tempSettings.globalSettings.sensitivity.resize(6, 0.0f);
            tempSettings.globalSettings.aspect_ratio = 0;
            tempSettings.globalSettings.fov = 82.0f;

            // Aimbot data defaults
            tempSettings.aimbotData.type = 0;
            tempSettings.aimbotData.enabled = false;
            tempSettings.aimbotData.maxDistance = 10;
            tempSettings.aimbotData.aimFov = 10;
            tempSettings.aimbotData.limitDetectorFps = true;
            tempSettings.aimbotData.verticalCorrection = 80;

            // PID settings defaults
            tempSettings.aimbotData.pidSettings.pidPreset = 0;
            tempSettings.aimbotData.pidSettings.proportional = 0.005f;
            tempSettings.aimbotData.pidSettings.integral = 0.05f;
            tempSettings.aimbotData.pidSettings.derivative = 0.008f;
            tempSettings.aimbotData.pidSettings.rampUpTime = 0.0f;

            // Colour aimbot settings defaults
            tempSettings.aimbotData.colourAimbotSettings.detectionPreset = 0;
            tempSettings.aimbotData.colourAimbotSettings.maxTrackAge = 3;
            tempSettings.aimbotData.colourAimbotSettings.trackSmoothingFactor = 0;
            tempSettings.aimbotData.colourAimbotSettings.trackConfidenceRate = 20;
            tempSettings.aimbotData.colourAimbotSettings.maxClusterDistance = 10;
            tempSettings.aimbotData.colourAimbotSettings.maxClusterDensityDifferential = 80;
            tempSettings.aimbotData.colourAimbotSettings.minDensity = 10;
            tempSettings.aimbotData.colourAimbotSettings.minArea = 5;
            tempSettings.aimbotData.colourAimbotSettings.aimHeight = -5;
            tempSettings.aimbotData.colourAimbotSettings.debugView = false;

            // AI aimbot settings defaults
            tempSettings.aimbotData.aiAimbotSettings.provider = 0;
            tempSettings.aimbotData.aiAimbotSettings.hitbox = 0;
            tempSettings.aimbotData.aiAimbotSettings.confidence = 10;
            tempSettings.aimbotData.aiAimbotSettings.forceHitbox = false;

            // Add triggerbot settings defaults
            tempSettings.aimbotData.triggerSettings.detectionMethod = 0;
            tempSettings.aimbotData.triggerSettings.sensitivity = 15.0f;
            tempSettings.aimbotData.triggerSettings.radius = 5;
            tempSettings.aimbotData.triggerSettings.showDebug = false;
			tempSettings.aimbotData.triggerSettings.sleepTime = 200;
			tempSettings.aimbotData.triggerSettings.burstDuration = 100;

            // Misc settings defaults
            tempSettings.misc.aimKeyMode = 0;
            tempSettings.misc.recoilKeyMode = 0;
            tempSettings.misc.quickPeekDelay = 100;

            // Open and read the original file to determine the game mode
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << xorstr_("Error opening file: ") << filename << std::endl;
                continue;
            }

            // Read the file once to determine the game/mode type
            std::string mode;
            std::string game;
            bool potato = false;
            std::vector<float> sensitivity;
            int aspect_ratio = 0;
            float fov = 82.0f;

            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string key, value;
                std::getline(iss, key, '=');
                std::getline(iss, value);

                if (key == xorstr_("Mode")) {
                    mode = value;
                }
                else if (key == xorstr_("Game")) {
                    game = value;
                }
                else if (key == xorstr_("Potato")) {
                    potato = (value == xorstr_("1"));
                }
                else if (key == xorstr_("Sensitivity")) {
                    std::istringstream ss(value);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        if (!item.empty()) {
                            sensitivity.push_back(std::stof(item));
                        }
                    }
                }
                else if (key == xorstr_("AspectRatio")) {
                    aspect_ratio = std::stoi(value);
                }
                else if (key == xorstr_("Fov")) {
                    fov = std::stof(value);
                }
            }
            file.close();

            // Setup global settings based on the detected game/mode
            tempSettings.globalSettings.potato = potato;
            tempSettings.globalSettings.aspect_ratio = aspect_ratio;
            tempSettings.globalSettings.fov = fov;

            // Set the sensitivity calculator based on the game
            if (game == xorstr_("Siege")) {
                tempSettings.globalSettings.sensitivityCalculator = 1; // Siege
            }
            else if (game == xorstr_("Rust")) {
                tempSettings.globalSettings.sensitivityCalculator = 2; // Rust
            }
            else if (game == xorstr_("Overwatch")) {
                tempSettings.globalSettings.sensitivityCalculator = 3; // Overwatch
            }
            else {
                tempSettings.globalSettings.sensitivityCalculator = 0; // Generic
            }

            // Set up detectors based on the game
            if (game == xorstr_("Siege")) {
                // For siege, enable operator detection and weapon detection
                tempSettings.globalSettings.characterDetectors[0] = true;
                tempSettings.globalSettings.weaponDetectors[0] = true;
            }
            else if (game == xorstr_("Rust")) {
                // For rust, enable weapon detection
                tempSettings.globalSettings.weaponDetectors[1] = true;
            }

            // Copy the sensitivity settings
            if (!sensitivity.empty()) {
                tempSettings.globalSettings.sensitivity = sensitivity;
            }

            // Ensure sensitivity has the correct size
            if (tempSettings.globalSettings.sensitivity.size() < 6) {
                tempSettings.globalSettings.sensitivity.resize(6, 0.0f);

                // Fill with sensible defaults based on game type
                if (game == xorstr_("Siege") && tempSettings.globalSettings.sensitivity.size() >= 6) {
                    // Siege default sensitivities if not provided
                    if (sensitivity.empty() || sensitivity.size() < 6) {
                        tempSettings.globalSettings.sensitivity[0] = 50.0f;  // X Base
                        tempSettings.globalSettings.sensitivity[1] = 50.0f;  // Y Base
                        tempSettings.globalSettings.sensitivity[2] = 100.0f; // 1x
                        tempSettings.globalSettings.sensitivity[3] = 70.0f;  // 2.5x
                        tempSettings.globalSettings.sensitivity[4] = 50.0f;  // 3.5x
                        tempSettings.globalSettings.sensitivity[5] = 0.02f;  // Multiplier
                    }
                }
                else if (game == xorstr_("Rust") && tempSettings.globalSettings.sensitivity.size() >= 2) {
                    // Rust only needs 2 sensitivities
                    if (sensitivity.empty() || sensitivity.size() < 2) {
                        tempSettings.globalSettings.sensitivity[0] = 0.509f; // Base Sensitivity
                        tempSettings.globalSettings.sensitivity[1] = 1.0f;   // ADS Sensitivity
                    }
                }
            }

            // Now process the actual character/weapon data based on the game type
            file.open(filename);

            // Variables to track what we're parsing
            std::vector<characterData> characters;
            characterData currentCharacter;
            weaponData currentWeapon;
            bool hasCharacters = false;
            bool parsingWeapon = false;

            // For configs without characters (like Rust)
            characterData defaultCharacter;
            defaultCharacter.charactername = xorstr_("Default Character");
            defaultCharacter.options.resize(5, false);

            // Process the file line by line
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string key, value;
                std::getline(iss, key, '=');
                std::getline(iss, value);

                // Parse AimAssist settings
                if (key == xorstr_("AimAssist")) {
                    std::istringstream ss(value);
                    std::string item;
                    std::vector<std::string> aimAssistValues;

                    while (std::getline(ss, item, ',')) {
                        aimAssistValues.push_back(item);
                    }

                    if (aimAssistValues.size() >= 5) {
                        tempSettings.aimbotData.type = std::stoi(aimAssistValues[0]);
                        tempSettings.aimbotData.maxDistance = std::stoi(aimAssistValues[1]);
                        tempSettings.aimbotData.aimFov = std::stoi(aimAssistValues[2]);

                        // Move triggerSleep to new location
                        tempSettings.aimbotData.triggerSettings.sleepTime = std::stoi(aimAssistValues[4]);

                        if (aimAssistValues.size() >= 6) {
                            tempSettings.aimbotData.limitDetectorFps = aimAssistValues[5] == xorstr_("1");
                        }
                        if (aimAssistValues.size() >= 7) {
                            // Move triggerBurstDuration to new location
                            tempSettings.aimbotData.triggerSettings.burstDuration = std::stoi(aimAssistValues[6]);
                        }
                        if (aimAssistValues.size() >= 8) {
                            tempSettings.aimbotData.verticalCorrection = std::stoi(aimAssistValues[7]);
                        }
                    }
                }
                // Parse PID settings
                else if (key == xorstr_("PidSettings")) {
                    std::istringstream ss(value);
                    std::string item;
                    std::vector<std::string> pidValues;

                    while (std::getline(ss, item, ',')) {
                        pidValues.push_back(item);
                    }

                    if (pidValues.size() >= 5) {
                        tempSettings.aimbotData.pidSettings.pidPreset = std::stoi(pidValues[0]);
                        tempSettings.aimbotData.pidSettings.proportional = std::stof(pidValues[1]);
                        tempSettings.aimbotData.pidSettings.integral = std::stof(pidValues[2]);
                        tempSettings.aimbotData.pidSettings.derivative = std::stof(pidValues[3]);
                        tempSettings.aimbotData.pidSettings.rampUpTime = std::stof(pidValues[4]);
                    }
                }
                // Parse ColourAimbot settings
                else if (key == xorstr_("ColourAimbotSettings")) {
                    std::istringstream ss(value);
                    std::string item;
                    std::vector<std::string> colourValues;

                    while (std::getline(ss, item, ',')) {
                        colourValues.push_back(item);
                    }

                    if (colourValues.size() >= 10) {
                        tempSettings.aimbotData.colourAimbotSettings.detectionPreset = std::stoi(colourValues[0]);
                        tempSettings.aimbotData.colourAimbotSettings.maxTrackAge = std::stoi(colourValues[1]);
                        tempSettings.aimbotData.colourAimbotSettings.trackSmoothingFactor = std::stoi(colourValues[2]);
                        tempSettings.aimbotData.colourAimbotSettings.trackConfidenceRate = std::stoi(colourValues[3]);
                        tempSettings.aimbotData.colourAimbotSettings.maxClusterDistance = std::stoi(colourValues[4]);
                        tempSettings.aimbotData.colourAimbotSettings.maxClusterDensityDifferential = std::stoi(colourValues[5]);
                        tempSettings.aimbotData.colourAimbotSettings.minDensity = std::stoi(colourValues[6]);
                        tempSettings.aimbotData.colourAimbotSettings.minArea = std::stoi(colourValues[7]);
                        tempSettings.aimbotData.colourAimbotSettings.aimHeight = std::stoi(colourValues[8]);
                        tempSettings.aimbotData.colourAimbotSettings.debugView = colourValues[9] == xorstr_("1");
                    }
                }
                // Parse AiAimbot settings
                else if (key == xorstr_("AiAimbotSettings")) {
                    std::istringstream ss(value);
                    std::string item;
                    std::vector<std::string> aiValues;

                    while (std::getline(ss, item, ',')) {
                        aiValues.push_back(item);
                    }

                    if (aiValues.size() >= 4) {
                        tempSettings.aimbotData.aiAimbotSettings.provider = std::stoi(aiValues[0]);
                        tempSettings.aimbotData.aiAimbotSettings.hitbox = std::stoi(aiValues[1]);
                        tempSettings.aimbotData.aiAimbotSettings.confidence = std::stoi(aiValues[2]);
                        tempSettings.aimbotData.aiAimbotSettings.forceHitbox = aiValues[3] == xorstr_("1");
                    }
                }
                // Parse Misc settings
                else if (key == xorstr_("Misc")) {
                    std::istringstream ss(value);
                    std::string item;
                    std::vector<std::string> miscValues;

                    while (std::getline(ss, item, ',')) {
                        miscValues.push_back(item);
                    }

                    if (miscValues.size() >= 3) {
                        tempSettings.misc.aimKeyMode = std::stoi(miscValues[0]);
                        tempSettings.misc.recoilKeyMode = std::stoi(miscValues[1]);
                        tempSettings.misc.quickPeekDelay = std::stoi(miscValues[2]);
                    }
                }
                // Parse Hotkey settings
                else if (key.substr(0, 6) == "Hotkey") {
                    int index = std::stoi(key.substr(6));
                    if (index >= 0 && index < static_cast<size_t>(HotkeyIndex::COUNT)) {
                        std::vector<int> hotkeyData;
                        std::istringstream valuess(value);
                        std::string token;

                        while (std::getline(valuess, token, ',')) {
                            hotkeyData.push_back(std::stoi(token));
                        }

                        if (hotkeyData.size() >= 3) {
                            tempSettings.misc.hotkeys.hotkeys[index].type.store(static_cast<InputType>(hotkeyData[0]));
                            tempSettings.misc.hotkeys.hotkeys[index].vKey.store(hotkeyData[1]);
                            tempSettings.misc.hotkeys.hotkeys[index].mode.store(static_cast<HotkeyMode>(hotkeyData[2]));
                        }
                    }
                }
                // Start of character data
                else if (key == xorstr_("Character")) {
                    if (!currentCharacter.charactername.empty()) {
                        // Save the previous character's current weapon if any
                        if (!currentWeapon.weaponname.empty()) {
                            // Make sure the weapon has attachments for the new format
                            if (currentWeapon.attachments.size() < 3) {
                                currentWeapon.attachments.resize(3, 0);
                            }
                            currentCharacter.weapondata.push_back(currentWeapon);
                            currentWeapon = weaponData();
                        }

                        // Add default selected weapons if missing
                        if (currentCharacter.selectedweapon.empty() && !currentCharacter.weapondata.empty()) {
                            if (currentCharacter.weapondata.size() >= 2) {
                                currentCharacter.selectedweapon = { 0, 1 };
                            }
                            else {
                                currentCharacter.selectedweapon = { 0, 0 };
                            }
                        }

                        // Validate selected weapon indices
                        if (!currentCharacter.weapondata.empty() && currentCharacter.selectedweapon.size() >= 2) {
                            size_t maxIndex = currentCharacter.weapondata.size() - 1;
                            if (currentCharacter.selectedweapon[0] > maxIndex) {
                                currentCharacter.selectedweapon[0] = 0;
                            }
                            if (currentCharacter.selectedweapon[1] > maxIndex) {
                                currentCharacter.selectedweapon[1] = 0;
                            }
                        }

                        characters.push_back(currentCharacter);
                    }

                    // Start a new character
                    currentCharacter = characterData();
                    currentCharacter.charactername = value;
                    // Make sure the character has options
                    currentCharacter.options.resize(5, false);
                    hasCharacters = true;
                    parsingWeapon = false;
                }
                // Character options
                else if (key == xorstr_("Options") && !parsingWeapon) {
                    currentCharacter.options.clear();
                    std::istringstream ss(value);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        currentCharacter.options.push_back(item == xorstr_("1"));
                    }
                    // Make sure we have enough options
                    if (currentCharacter.options.size() < 5) {
                        currentCharacter.options.resize(5, false);
                    }
                }
                // Selected weapons for character
                else if (key == xorstr_("SelectedWeapons") && !parsingWeapon) {
                    currentCharacter.selectedweapon.clear();
                    std::istringstream ss(value);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        if (!item.empty()) {
                            currentCharacter.selectedweapon.push_back(std::stoi(item));
                        }
                    }
                    if (currentCharacter.selectedweapon.size() < 2) {
                        currentCharacter.selectedweapon.resize(2, 0);
                    }
                }
                // Weapon data - for both character-based and non-character configs
                else if (key == xorstr_("Weapon")) {
                    if (!currentWeapon.weaponname.empty()) {
                        // Make sure the weapon has attachments for the new format
                        if (currentWeapon.attachments.size() < 3) {
                            currentWeapon.attachments.resize(3, 0);
                        }

                        if (hasCharacters) {
                            // If we're parsing characters, add to current character
                            currentCharacter.weapondata.push_back(currentWeapon);
                        }
                        else {
                            // If no characters, add to default character
                            defaultCharacter.weapondata.push_back(currentWeapon);
                        }
                    }

                    currentWeapon = weaponData();
                    currentWeapon.weaponname = value;
                    // Initialize with default attachments
                    currentWeapon.attachments.resize(3, 0);
                    parsingWeapon = true;
                }
                // Weapon rapid fire
                else if ((key == xorstr_("RapidFire") || key == xorstr_("AutoFire")) && parsingWeapon) {
                    currentWeapon.rapidfire = (value == xorstr_("1"));
                }
                // Weapon attachments
                else if (key == xorstr_("Attachments") && parsingWeapon) {
                    currentWeapon.attachments.clear();
                    std::istringstream ss(value);
                    std::string item;
                    while (std::getline(ss, item, ',')) {
                        if (!item.empty()) {
                            currentWeapon.attachments.push_back(std::stoi(item));
                        }
                    }
                    // Ensure we have 3 attachments
                    if (currentWeapon.attachments.size() < 3) {
                        currentWeapon.attachments.resize(3, 0);
                    }
                }
                // Weapon recoil values
                else if (key == xorstr_("Values") && parsingWeapon) {
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

                    // If the weapon has no recoil values, add a default pattern
                    if (currentWeapon.values.empty()) {
                        currentWeapon.values = {
                            {0.1f, 0.1f, 1000.0f},
                            {0.2f, 0.2f, 1000.0f},
                            {0.1f, 0.3f, 1000.0f}
                        };
                    }
                }
            }
            file.close();

            // Handle the last weapon and character
            if (!currentWeapon.weaponname.empty()) {
                // Make sure the weapon has attachments for the new format
                if (currentWeapon.attachments.size() < 3) {
                    currentWeapon.attachments.resize(3, 0);
                }

                if (hasCharacters) {
                    currentCharacter.weapondata.push_back(currentWeapon);
                }
                else {
                    defaultCharacter.weapondata.push_back(currentWeapon);
                }
            }

            if (!currentCharacter.charactername.empty()) {
                // Add default selected weapons if missing
                if (currentCharacter.selectedweapon.empty() && !currentCharacter.weapondata.empty()) {
                    if (currentCharacter.weapondata.size() >= 2) {
                        currentCharacter.selectedweapon = { 0, 1 };
                    }
                    else {
                        currentCharacter.selectedweapon = { 0, 0 };
                    }
                }

                // Validate selected weapon indices
                if (!currentCharacter.weapondata.empty() && currentCharacter.selectedweapon.size() >= 2) {
                    size_t maxIndex = currentCharacter.weapondata.size() - 1;
                    if (currentCharacter.selectedweapon[0] > maxIndex) {
                        currentCharacter.selectedweapon[0] = 0;
                    }
                    if (currentCharacter.selectedweapon[1] > maxIndex) {
                        currentCharacter.selectedweapon[1] = 0;
                    }
                }

                characters.push_back(currentCharacter);
            }

            // For non-character configs, use the default character if weapons were found
            if (!hasCharacters && !defaultCharacter.weapondata.empty()) {
                // Add default selected weapons
                if (defaultCharacter.selectedweapon.empty()) {
                    if (defaultCharacter.weapondata.size() >= 2) {
                        defaultCharacter.selectedweapon = { 0, 1 };
                    }
                    else {
                        defaultCharacter.selectedweapon = { 0, 0 };
                    }
                }

                characters.push_back(defaultCharacter);
            }

            // If no characters were created at all, create a default one
            if (characters.empty()) {
                characterData emptyChar;
                emptyChar.charactername = xorstr_("Default Character");
                emptyChar.options.resize(5, false);

                // Create primary weapon
                weaponData primaryWeapon;
                primaryWeapon.weaponname = xorstr_("Primary Weapon");
                primaryWeapon.rapidfire = false;
                primaryWeapon.attachments = { 0, 0, 0 };
                primaryWeapon.values = { {0.1f, 0.1f, 1000.0f}, {0.2f, 0.2f, 1000.0f}, {0.1f, 0.3f, 1000.0f} };

                // Create secondary weapon
                weaponData secondaryWeapon;
                secondaryWeapon.weaponname = xorstr_("Secondary Weapon");
                secondaryWeapon.rapidfire = false;
                secondaryWeapon.attachments = { 0, 0, 0 };
                secondaryWeapon.values = { {0.05f, 0.05f, 1000.0f}, {0.1f, 0.1f, 1000.0f} };

                // Add weapons to character
                emptyChar.weapondata.push_back(primaryWeapon);
                emptyChar.weapondata.push_back(secondaryWeapon);
                emptyChar.selectedweapon = { 0, 1 };

                characters.push_back(emptyChar);
            }

            // Make another pass through all characters to ensure they all have properly initialized weapons
            for (auto& character : characters) {
                for (auto& weapon : character.weapondata) {
                    // Ensure attachments are present
                    if (weapon.attachments.size() < 3) {
                        weapon.attachments.resize(3, 0);
                    }

                    // Ensure values are present
                    if (weapon.values.empty()) {
                        weapon.values = {
                            {0.1f, 0.1f, 1000.0f},
                            {0.2f, 0.2f, 1000.0f},
                            {0.1f, 0.3f, 1000.0f}
                        };
                    }
                }

                // Ensure character has options
                if (character.options.size() < 5) {
                    character.options.resize(5, false);
                }

                // Ensure character has selected weapons
                if (character.selectedweapon.empty() && !character.weapondata.empty()) {
                    if (character.weapondata.size() >= 2) {
                        character.selectedweapon = { 0, 1 };
                    }
                    else {
                        character.selectedweapon = { 0, 0 };
                    }
                }
            }

            // Set the characters in the new settings
            tempSettings.characters = characters;

            // Set active state
            tempSettings.activeState.selectedCharacterIndex = 0;
            tempSettings.activeState.isPrimaryActive = true;
            tempSettings.activeState.weaponOffOverride = false;
            tempSettings.activeState.weaponDataChanged = true;
            tempSettings.activeState.pidDataChanged = true;
            tempSettings.activeState.sensMultiplier = { 1.0f, 1.0f };

            // Save the new config to the original filename
            tempSettings.saveSettings(filename);
            convertedFiles.push_back(filename);
        }
    }

    return convertedFiles;
}

void Settings::readSettings(const std::string& filename, bool clearExisting, bool updateAimbotInfo) {
    // Check if this is a legacy config and convert if needed
    if (isLegacyConfig(filename)) {
        // Don't load legacy configs
        return;
    }

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

        // Global settings
        if (key == xorstr_("Potato")) {
            globalSettings.potato = (value == xorstr_("1"));
        }
        else if (key == xorstr_("SensitivityCalculator")) {
            globalSettings.sensitivityCalculator = std::stoi(value);
        }
        else if (key == xorstr_("CharacterDetectors")) {
            globalSettings.characterDetectors.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                globalSettings.characterDetectors.push_back(item == xorstr_("1"));
            }
        }
        else if (key == xorstr_("WeaponDetectors")) {
            globalSettings.weaponDetectors.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                globalSettings.weaponDetectors.push_back(item == xorstr_("1"));
            }
        }
        else if (key == xorstr_("Sensitivity")) {
            globalSettings.sensitivity.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                if (!item.empty()) {
                    globalSettings.sensitivity.push_back(std::stof(item));
                }
            }
        }
        else if (key == xorstr_("AspectRatio")) {
            globalSettings.aspect_ratio = std::stoi(value);
        }
        else if (key == xorstr_("Fov")) {
            globalSettings.fov = std::stof(value);
        }
        // Aimbot settings
        else if (key == xorstr_("AimAssist") && updateAimbotInfo) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); aimbotData.type = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.maxDistance = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.aimFov = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.limitDetectorFps = item == xorstr_("1");
            std::getline(ss, item, ','); aimbotData.verticalCorrection = std::stoi(item);
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
        else if (key == xorstr_("ColourAimbotSettings")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.detectionPreset = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.maxTrackAge = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.trackSmoothingFactor = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.trackConfidenceRate = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.maxClusterDistance = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.maxClusterDensityDifferential = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.minDensity = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.minArea = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.aimHeight = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.colourAimbotSettings.debugView = item == xorstr_("1");
        }
        else if (key == xorstr_("AiAimbotSettings")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); aimbotData.aiAimbotSettings.provider = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.aiAimbotSettings.hitbox = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.aiAimbotSettings.confidence = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.aiAimbotSettings.forceHitbox = item == xorstr_("1");
        }
        else if (key == xorstr_("TriggerBotSettings")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); aimbotData.triggerSettings.detectionMethod = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.triggerSettings.sensitivity = std::stof(item);
            std::getline(ss, item, ','); aimbotData.triggerSettings.radius = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.triggerSettings.showDebug = item == xorstr_("1");
            std::getline(ss, item, ','); aimbotData.triggerSettings.sleepTime = std::stoi(item);
            std::getline(ss, item, ','); aimbotData.triggerSettings.burstDuration = std::stoi(item);
        }
        // Misc settings
        else if (key == xorstr_("Misc")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); misc.aimKeyMode = std::stoi(item);
            std::getline(ss, item, ','); misc.recoilKeyMode = std::stoi(item);
            std::getline(ss, item, ','); misc.quickPeekDelay = std::stoi(item);
        }
        // Hotkeys
        else if (key.substr(0, 6) == "Hotkey") {
            int index = std::stoi(key.substr(6));
            if (index >= 0 && index < static_cast<size_t>(HotkeyIndex::COUNT)) {
                std::vector<int> hotkeyData;
                std::istringstream valuess(value);
                std::string token;

                while (std::getline(valuess, token, ',')) {
                    hotkeyData.push_back(std::stoi(token));
                }

                if (hotkeyData.size() >= 3) {
                    misc.hotkeys.hotkeys[index].type.store(static_cast<InputType>(hotkeyData[0]));
                    misc.hotkeys.hotkeys[index].vKey.store(hotkeyData[1]);
                    misc.hotkeys.hotkeys[index].mode.store(static_cast<HotkeyMode>(hotkeyData[2]));
                }
            }
        }
        // Overlay
        else if (key == xorstr_("Overlay")) {
            std::istringstream ss(value);
            std::string item;
            std::getline(ss, item, ','); misc.overlay.enabled = item == xorstr_("1");
            std::getline(ss, item, ','); misc.overlay.showInfo = item == xorstr_("1");
            std::getline(ss, item, ','); misc.overlay.magnifierZoom = std::stoi(item);
            std::getline(ss, item, ','); misc.overlay.magnifierSize = std::stoi(item);
        }
        // Character data
        else if (key == xorstr_("Character")) {
            if (!currentCharacter.charactername.empty()) {
                if (!currentWeapon.weaponname.empty()) {
                    currentCharacter.weapondata.push_back(currentWeapon);
                    currentWeapon = weaponData();
                }
                // Validate selected weapon indices before adding the character
                if (!currentCharacter.weapondata.empty()) {
                    size_t maxWeaponIndex = currentCharacter.weapondata.size() - 1;
                    if (currentCharacter.selectedweapon.size() >= 2) {
                        if (currentCharacter.selectedweapon[0] > maxWeaponIndex) {
                            currentCharacter.selectedweapon[0] = 0;
                        }
                        if (currentCharacter.selectedweapon[1] > maxWeaponIndex) {
                            currentCharacter.selectedweapon[1] = 0;
                        }
                    }
                    else {
                        currentCharacter.selectedweapon = { 0, 0 };
                    }
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
            currentCharacter.selectedweapon.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                currentCharacter.selectedweapon.push_back(std::stoi(item));
            }
            if (currentCharacter.selectedweapon.size() < 2) {
                currentCharacter.selectedweapon.resize(2, 0);
            }
        }
        // Weapon data
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
            currentWeapon.attachments.clear();
            std::istringstream ss(value);
            std::string item;
            while (std::getline(ss, item, ',')) {
                currentWeapon.attachments.push_back(std::stoi(item));
            }
            if (currentWeapon.attachments.size() < 3) {
                currentWeapon.attachments.resize(3, 0);
            }
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
    }

    // Add the last weapon and character
    if (!currentWeapon.weaponname.empty()) {
        currentCharacter.weapondata.push_back(currentWeapon);
        weaponCount++;
    }
    if (!currentCharacter.charactername.empty()) {
        // Validate selected weapon indices for the last character
        if (!currentCharacter.weapondata.empty()) {
            size_t maxWeaponIndex = currentCharacter.weapondata.size() - 1;
            if (currentCharacter.selectedweapon.size() >= 2) {
                if (currentCharacter.selectedweapon[0] > maxWeaponIndex) {
                    currentCharacter.selectedweapon[0] = 0;
                }
                if (currentCharacter.selectedweapon[1] > maxWeaponIndex) {
                    currentCharacter.selectedweapon[1] = 0;
                }
            }
            else {
                currentCharacter.selectedweapon = { 0, 0 };
            }
        }
        characters.push_back(currentCharacter);
    }

    // Set the initial selected character and weapon
    if (!characters.empty()) {
        activeState.selectedCharacterIndex = 0;
        activeState.isPrimaryActive = true;
    }

    // Set the weaponDataChanged flag
    activeState.weaponDataChanged = true;
    activeState.pidDataChanged = true;
}

void Settings::saveSettings(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << xorstr_("Error opening file for writing: ") << filename << std::endl;
        return;
    }

    // Save global settings
    file << xorstr_("Potato=") << (globalSettings.potato ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
    file << xorstr_("SensitivityCalculator=") << globalSettings.sensitivityCalculator << xorstr_("\n");

    // Save detectors
    file << xorstr_("CharacterDetectors=");
    for (const auto& detector : globalSettings.characterDetectors) {
        file << (detector ? xorstr_("1") : xorstr_("0")) << xorstr_(",");
    }
    file << xorstr_("\n");

    file << xorstr_("WeaponDetectors=");
    for (const auto& detector : globalSettings.weaponDetectors) {
        file << (detector ? xorstr_("1") : xorstr_("0")) << xorstr_(",");
    }
    file << xorstr_("\n");

    // Save sensitivity settings
    file << xorstr_("Sensitivity=");
    for (const auto& sens : globalSettings.sensitivity) {
        file << sens << xorstr_(",");
    }
    file << xorstr_("\n");

    file << xorstr_("AspectRatio=") << globalSettings.aspect_ratio << xorstr_("\n");
    file << xorstr_("Fov=") << globalSettings.fov << xorstr_("\n");

    // Save aimbot settings
    file << xorstr_("AimAssist=") 
        << aimbotData.type << xorstr_(",") 
        << aimbotData.maxDistance << xorstr_(",") 
        << aimbotData.aimFov << xorstr_(",") 
        << (aimbotData.limitDetectorFps ? xorstr_("1") : xorstr_("0")) << xorstr_(",") 
        << aimbotData.verticalCorrection << xorstr_("\n");

    file << xorstr_("PidSettings=") 
        << aimbotData.pidSettings.pidPreset << xorstr_(",") 
        << aimbotData.pidSettings.proportional << xorstr_(",") 
        << aimbotData.pidSettings.integral << xorstr_(",") 
        << aimbotData.pidSettings.derivative << xorstr_(",")
        << aimbotData.pidSettings.rampUpTime << xorstr_("\n");

    file << xorstr_("ColourAimbotSettings=") 
        << aimbotData.colourAimbotSettings.detectionPreset << xorstr_(",") 
        << aimbotData.colourAimbotSettings.maxTrackAge << xorstr_(",") 
        << aimbotData.colourAimbotSettings.trackSmoothingFactor << xorstr_(",") 
        << aimbotData.colourAimbotSettings.trackConfidenceRate << xorstr_(",") 
        << aimbotData.colourAimbotSettings.maxClusterDistance << xorstr_(",") 
        << aimbotData.colourAimbotSettings.maxClusterDensityDifferential << xorstr_(",") 
        << aimbotData.colourAimbotSettings.minDensity << xorstr_(",") 
        << aimbotData.colourAimbotSettings.minArea << xorstr_(",")
        << aimbotData.colourAimbotSettings.aimHeight << xorstr_(",") 
        << (aimbotData.colourAimbotSettings.debugView ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
    
    file << xorstr_("AiAimbotSettings=") 
        << aimbotData.aiAimbotSettings.provider << xorstr_(",") 
        << aimbotData.aiAimbotSettings.hitbox << xorstr_(",") 
        << aimbotData.aiAimbotSettings.confidence << xorstr_(",") 
        << (aimbotData.aiAimbotSettings.forceHitbox ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");
    
    file << xorstr_("TriggerBotSettings=")
        << settings.aimbotData.triggerSettings.detectionMethod << xorstr_(",")
        << settings.aimbotData.triggerSettings.sensitivity << xorstr_(",")
        << settings.aimbotData.triggerSettings.radius << xorstr_(",")
        << (settings.aimbotData.triggerSettings.showDebug ? xorstr_("1") : xorstr_("0")) << xorstr_(",")
        << settings.aimbotData.triggerSettings.sleepTime << xorstr_(",")
        << settings.aimbotData.triggerSettings.burstDuration << xorstr_("\n");
    
    file << xorstr_("Misc=") 
        << misc.aimKeyMode << xorstr_(",") 
        << misc.recoilKeyMode << xorstr_(",") 
        << misc.quickPeekDelay << xorstr_("\n");

    // Save hotkeys in matching format
    for (size_t i = 0; i < static_cast<size_t>(HotkeyIndex::COUNT); i++) {
        const Hotkey& hotkey = misc.hotkeys.hotkeys[i];
        file << "Hotkey" << i << "=" <<
            static_cast<int>(hotkey.type.load()) << "," <<
            hotkey.vKey.load() << "," <<
            static_cast<int>(hotkey.mode.load()) << "\n";
    }

    file << xorstr_("Overlay=")
        << (misc.overlay.enabled ? xorstr_("1") : xorstr_("0")) << xorstr_(",")
        << (misc.overlay.showInfo ? xorstr_("1") : xorstr_("0")) << xorstr_(",")
        << misc.overlay.magnifierZoom << xorstr_(",")
        << misc.overlay.magnifierSize << xorstr_("\n");

    // Save character and weapon data
    for (const auto& character : characters) {
        file << xorstr_("Character=") << character.charactername << xorstr_("\n");
        file << xorstr_("Options=");
        for (const auto& option : character.options) {
            file << (option ? xorstr_("1") : xorstr_("0")) << xorstr_(",");
        }
        file << xorstr_("\n");
        file << xorstr_("SelectedWeapons=");
        for (const auto& weaponIndex : character.selectedweapon) {
            file << weaponIndex << xorstr_(",");
        }
        file << xorstr_("\n");

        for (const auto& weapon : character.weapondata) {
            file << xorstr_("Weapon=") << weapon.weaponname << xorstr_("\n");
            file << xorstr_("RapidFire=") << (weapon.rapidfire ? xorstr_("1") : xorstr_("0")) << xorstr_("\n");

            // Only save attachments if there are any
            if (!weapon.attachments.empty()) {
                file << xorstr_("Attachments=");
                for (const auto& attachment : weapon.attachments) {
                    file << attachment << xorstr_(",");
                }
                file << xorstr_("\n");
            }

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