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
            std::getline(ss, item, ','); misc.overlay.showInfo = item == xorstr_("1");
            std::getline(ss, item, ','); misc.overlay.magnifierZoom = std::stof(item);
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
        else if (key == xorstr_("TriggerBurstDuration")) {
            currentWeapon.triggerBurstDuration = std::stoi(value);
    }
        else if (key == xorstr_("TriggerFireDelay")) {
            currentWeapon.triggerFireDelay = std::stoi(value);
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
        << settings.aimbotData.triggerSettings.sleepTime << xorstr_("\n");
    
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

            file << xorstr_("TriggerBurstDuration=") << weapon.triggerBurstDuration << xorstr_("\n");
            file << xorstr_("TriggerFireDelay=") << weapon.triggerFireDelay << xorstr_("\n");
        }
    }
}