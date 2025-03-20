#include "menu.hpp"

Settings cfg;

bool Menu::comboBoxGen(const char* label, int* current_item, const char* const items[], int items_count) {
	ImGui::Spacing();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
	ImGui::Text(label);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	return ImGui::Combo(std::string(xorstr_("##COMBOGEN__") + std::string(label)).c_str(), current_item, items, items_count);
}

bool Menu::comboBoxChar(const char* label, int& characterIndex, const std::vector<characterData>& items) {
	ImGui::Spacing();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
	ImGui::Text(label);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
	if (ImGui::BeginCombo(std::string(xorstr_("##COMBOCHAR__") + std::string(label)).c_str(), items[characterIndex].charactername.c_str())) {
        for (int i = 0; i < items.size(); i++) {
            const bool isSelected = (characterIndex == i);
            if (ImGui::Selectable(items[i].charactername.c_str(), isSelected)) {
                characterIndex = i;

                ImGui::SetItemDefaultFocus();
                ImGui::EndCombo();
                return true;
            }
        }
        ImGui::EndCombo();
    }
    return false;
}

bool Menu::comboBoxWep(const char* label, int& characterIndex, int& weaponIndex, const std::vector<characterData>& items) {
	//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
	//ImGui::Text(label);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
    if (ImGui::BeginCombo(std::string(xorstr_("##COMBOWEP__") + std::string(label)).c_str(), items[characterIndex].weapondata[weaponIndex].weaponname.c_str())) {
        for (int i = 0; i < items[characterIndex].weapondata.size(); i++) {
            const bool isSelected = (weaponIndex == i);
            if (ImGui::Selectable(items[characterIndex].weapondata[i].weaponname.c_str(), isSelected)) {
                weaponIndex = i; 

                ImGui::SetItemDefaultFocus();
                ImGui::EndCombo();
                return true;
            }
        }
        ImGui::EndCombo();
    }
    return false;
}

bool Menu::multiCombo(const char* label, std::vector<const char*>& items, std::vector<bool>& selected) {
    bool changed = false;
    std::string displayString;
    for (int i = 0; i < items.size(); ++i) {
        if (selected[i]) {
            if (!displayString.empty())
                displayString += ", ";
            displayString += items[i];
        }
    }

	ImGui::Spacing();
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 6);
	ImGui::Text(label);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);

    if (ImGui::BeginCombo(std::string(xorstr_("##MULTICOMBO__") + std::string(label)).c_str(), displayString.c_str())) {
        for (int i = 0; i < items.size(); ++i) {
            ImGui::PushID(i);
            bool isSelected = selected[i];
            if (ImGui::Selectable(items[i], &isSelected)) {
                selected[i] = !selected[i];
                changed = true;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    return changed;
}

void tooltip(const char* desc, bool showOnHover = true) {
	if (showOnHover && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort) || !showOnHover) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

		// Apply some styling to make the tooltip stand out
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::Text("%s", desc);
		ImGui::PopStyleColor();

		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void newConfigPopup(bool trigger, char* newConfigName) {
	if (trigger) {
		ImGuiViewport* viewport = ImGui::GetWindowViewport();
		ImVec2 center = viewport->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup(xorstr_("New Config"));
	}

	ImGuiWindowFlags popup_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Modal;

	if (ImGui::BeginPopupModal(xorstr_("New Config"), NULL, popup_flags)) {
		ImGui::Text(xorstr_("Enter a name for your new config:"));
		ImGui::InputText(xorstr_("Config Name"), newConfigName, 256);

		ImGui::Separator();

		if (ImGui::Button(xorstr_("Create"))) {
			std::string newFileName = std::string(newConfigName) + xorstr_(".focus");
			Settings newSettings;

			// Set default global settings
			newSettings.globalSettings.potato = true;
			newSettings.globalSettings.fov = 82.0f;
			newSettings.globalSettings.aspect_ratio = 0; // 16:9
			newSettings.globalSettings.sensitivityCalculator = 0; // Generic by default
			newSettings.globalSettings.characterDetectors.resize(1, false);
			newSettings.globalSettings.weaponDetectors.resize(2, false);
			newSettings.globalSettings.sensitivity.resize(6, 0.0f);

			// Default sensitivities
			newSettings.globalSettings.sensitivity[0] = 50.0f; // X Base
			newSettings.globalSettings.sensitivity[1] = 50.0f; // Y Base
			newSettings.globalSettings.sensitivity[2] = 100.0f; // 1x
			newSettings.globalSettings.sensitivity[3] = 100.0f; // 2.5x
			newSettings.globalSettings.sensitivity[4] = 100.0f; // 3.5x
			newSettings.globalSettings.sensitivity[5] = 0.02f; // Multiplier

			// Default aimbot settings
			newSettings.aimbotData = { 0, 0, false, 0, 10, 10, true, 80,
				{ 0, 0.02f, 0.2f, 0.008f, 1.f },
				{ 0, 3, 0, 20, 10, 80, 10, 5, -5, false },
				{ 0, 0, 10, false },
				{ 0, 15.0f, 5, false, 200, 100 }
			};

			// Create a default character
			characterData defaultChar;
			defaultChar.charactername = xorstr_("Default Character");
			defaultChar.options.resize(5, false);

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
			defaultChar.weapondata.push_back(primaryWeapon);
			defaultChar.weapondata.push_back(secondaryWeapon);
			defaultChar.selectedweapon = { 0, 1 };

			// Add character to settings
			newSettings.characters.push_back(defaultChar);

			// Set active state
			newSettings.activeState.selectedCharacterIndex = 0;
			newSettings.activeState.isPrimaryActive = true;
			newSettings.activeState.weaponOffOverride = false;

			// Save new config
			newSettings.saveSettings(newFileName);
			globals.filesystem.configFiles.push_back(newFileName);

			memset(newConfigName, 0, sizeof(char) * 256);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button(xorstr_("Cancel"))) {
			memset(newConfigName, 0, sizeof(char) * 256);
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

void renameConfigPopup(bool& trigger, static char renameBuffer[256]) {
	if (trigger) {
		ImGuiViewport* viewport = ImGui::GetWindowViewport();
		ImVec2 center = viewport->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup(xorstr_("Rename Config"));
	}

	if (ImGui::BeginPopupModal(xorstr_("Rename Config"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text(xorstr_("Rename config:"));
		ImGui::Text(xorstr_("%s"), globals.filesystem.configFiles[globals.filesystem.activeFileIndex].c_str());
		ImGui::InputText(xorstr_("New Name"), renameBuffer, IM_ARRAYSIZE(renameBuffer));
		ImGui::Separator();

		if (ImGui::Button(xorstr_("Rename"), ImVec2(120, 0))) {
			std::string oldName = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
			std::string newName = std::string(renameBuffer) + xorstr_(".focus");
			if (rename(oldName.c_str(), newName.c_str()) == 0) {
				globals.filesystem.configFiles[globals.filesystem.activeFileIndex] = newName;
				if (globals.filesystem.activeFile == oldName) {
					globals.filesystem.activeFile = newName;
				}
			}
			else {
				ImGui::OpenPopup(xorstr_("RnameErrorPopup"));
			}
			memset(renameBuffer, 0, sizeof(renameBuffer));
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Cancel"), ImVec2(120, 0))) {
			memset(renameBuffer, 0, sizeof(renameBuffer));
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void Menu::popup(bool& trigger, int type) {

	const char* chartype;

	switch (type) {
		case 0:
			chartype = xorstr_("Load");
			break;
		case 1:
			chartype = xorstr_("Save");
			break;
		case 2:
			chartype = xorstr_("Initiate Shutdown");
			break;
		case 3:
			chartype = xorstr_("Delete");
			break;
	}

	if (trigger) {
		ImGuiViewport* viewport = ImGui::GetWindowViewport();
		ImVec2 center = viewport->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::OpenPopup(chartype);
	}

	if (ImGui::BeginPopupModal(chartype)) {
		if (type == 0) {
			ImGui::Text(xorstr_("You have unsaved changes in the current config."));
			ImGui::Text(xorstr_("Loading a new config will discard these changes."));
		}
		else if (type == 1) {
			ImGui::Text(xorstr_("You are about to save over a different config file."));
			ImGui::Text(xorstr_("Current active config: %s"), globals.filesystem.activeFile.c_str());
			ImGui::Text(xorstr_("Selected config to save over: %s"), globals.filesystem.configFiles[globals.filesystem.activeFileIndex].c_str());
		}
		else if (type == 2) {
			ImGui::Text(xorstr_("You are about to initiate a shutdown with pending changes."));
			ImGui::Text(xorstr_("Doing so will discard these changes."));
		}
		else if (type == 3) {
			ImGui::Text(xorstr_("You are about to delete a config file."));
			ImGui::Text(xorstr_("Selected config to delete: %s"), globals.filesystem.configFiles[globals.filesystem.activeFileIndex].c_str());
			ImGui::Text(xorstr_("This action cannot be undone."));
		}

		ImGui::Separator();
		ImGui::Text(xorstr_("Are you sure you want to proceed?"));

		if (ImGui::Button(xorstr_("Yes"), ImVec2(120, 0))) {
			if (type == 0) {
				globals.filesystem.activeFile = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
				settings.activeState.selectedCharacterIndex = 0;
				settings.activeState.weaponOffOverride = false;
				settings.readSettings(globals.filesystem.activeFile, true, true);
				globals.filesystem.unsavedChanges = false;
			}
			else if (type == 1) {
				settings.saveSettings(globals.filesystem.configFiles[globals.filesystem.activeFileIndex]);
				globals.filesystem.unsavedChanges = false;
				globals.filesystem.activeFile = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
			}
			else if (type == 2) {
				globals.done = true;
			}
			else if (type == 3) {
				std::string fileToDelete = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
				remove(fileToDelete.c_str());
				globals.filesystem.configFiles.erase(globals.filesystem.configFiles.begin() + globals.filesystem.activeFileIndex);
				if (globals.filesystem.activeFile == fileToDelete) {
					globals.filesystem.activeFile = xorstr_("");
				}

				if (globals.filesystem.configFiles.empty()) {
					globals.filesystem.activeFileIndex = -1;
				}
				else if (globals.filesystem.activeFileIndex >= globals.filesystem.configFiles.size()) {
					globals.filesystem.activeFileIndex = globals.filesystem.configFiles.size() - 1;
				}
			}

			trigger = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("No"), ImVec2(120, 0))) {
			globals.initshutdown = false;
			trigger = false;
			ImGui::CloseCurrentPopup();
		}

		if (type == 2) {
			ImGui::SameLine();
			if (ImGui::Button(xorstr_("Save and Quit"), ImVec2(120, 0))) {
				settings.saveSettings(globals.filesystem.configFiles[globals.filesystem.activeFileIndex]);
				globals.filesystem.unsavedChanges = false;

				globals.done = true;
				trigger = false;
				ImGui::CloseCurrentPopup();
			}
		}

		ImGui::EndPopup();
	}
}

void Menu::startupchecks_gui() {
	ImGui::SetNextWindowSizeConstraints(ImVec2(350, 200), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin(xorstr_("Startup Checks"), NULL, STARTUPFLAGS);

    if (globals.startup.mouse_driver) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text(xorstr_("Mouse driver is running"));
        std::string str = ut.wstring_to_string(ms.findDriver());
        ImGui::Text(str.c_str());
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(xorstr_("Mouse driver is not running"));
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

	if (globals.startup.keyboard_driver) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		ImGui::Text(xorstr_("Keyboard driver is running"));
		std::string str = ut.wstring_to_string(kb.findDriver());
		ImGui::Text(str.c_str());
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text(xorstr_("Keyboard driver is not running"));
		ImGui::PopStyleColor();
	}

	ImGui::Separator();

    if (globals.startup.files) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text(xorstr_("Settings files found"));
        for (int i = 0; i < globals.filesystem.configFiles.size(); i++) {
            ImGui::Text(globals.filesystem.configFiles[i].c_str());
        }
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
        ImGui::Text(xorstr_("No settings files found"));
        ImGui::Text(xorstr_("Please create one and refresh from the file tab"));
        ImGui::PopStyleColor();
    }

	ImGui::Separator();

	if (globals.startup.dxgi) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		ImGui::Text(xorstr_("DXGI was initialised"));
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text(xorstr_("DXGI could not be initialised"));
		ImGui::Text(xorstr_("Please contact support"));
		ImGui::PopStyleColor();
	}

	ImGui::Separator();

	if (globals.startup.marker) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		ImGui::Text(xorstr_("Marker was generated"));
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text(xorstr_("Marker could not be generated"));
		ImGui::PopStyleColor();
	}

	ImGui::Separator();

	if (globals.startup.avx) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		if (globals.startup.avx == 1) {
			ImGui::Text(xorstr_("AVX2 available on this system"));
		}
		else if (globals.startup.avx == 2) {
			ImGui::Text(xorstr_("AVX512 available on this system"));
		}
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
		ImGui::Text(xorstr_("AVX not available on this system"));
		ImGui::PopStyleColor();
	}

    ImGui::End();
}

std::vector<float> calculateSensitivityModifierR6() {
	float oldBaseSens = 10;
	float oldRelativeSens = 50;
	float oldMultiplier = 0.02f;
	float oldFov = 82.0f;

	float newBaseXSens = settings.globalSettings.sensitivity[0];
	float newBaseYSens = settings.globalSettings.sensitivity[1];
	float new1xSens = settings.globalSettings.sensitivity[2];
	float new25xSens = settings.globalSettings.sensitivity[3];
	float new35xSens = settings.globalSettings.sensitivity[4];
	float newMultiplier = settings.globalSettings.sensitivity[5];

	float newFov = settings.globalSettings.fov;

	int activescope = 0;

	float sightXEffect = 1.0f;
	float sightYEffect = 1.0f;

	float gripEffect = 1.0f;

	float barrelXEffect = 1.0f;
	float barrelYEffect = 1.0f;

	float fovDifference = newFov - oldFov;
	float quadratic_coef = 0.0000153f;  // Solved with trig :gangster:
	float linear_coef = 0.0013043f;
	float fovModifier = 1.0f + (quadratic_coef * fovDifference * fovDifference) +
		(linear_coef * fovDifference);

	settings.activeState.fovSensitivityModifier = fovModifier;

	if (settings.activeState.selectedCharacterIndex < settings.characters.size() &&
		!settings.characters[settings.activeState.selectedCharacterIndex].weapondata.empty()) {

		int weaponIndex = settings.activeState.isPrimaryActive ?
			settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] :
			settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1];

		weaponIndex = std::min(weaponIndex, static_cast<int>(settings.characters[settings.activeState.selectedCharacterIndex].weapondata.size()) - 1);

		const auto& weapon = settings.characters[settings.activeState.selectedCharacterIndex].weapondata[weaponIndex];

		if (weapon.attachments.size() >= 3) {
			switch (weapon.attachments[0]) {
			case 0:
				sightXEffect = 1.0f;
				sightYEffect = 1.0f;
				activescope = 1;
				break;
			case 1:
				sightXEffect = 2.f;
				sightYEffect = 2.43f;
				activescope = 2;
				break;
			case 2:
				sightXEffect = 3.5f;
				sightYEffect = 3.5f;
				activescope = 3;
				break;
			}

			switch (weapon.attachments[1]) {
			case 0:
				gripEffect = 1.0f;
				break;
			case 1:
				gripEffect = 0.8f;
				break;
			}

			switch (weapon.attachments[2]) {
			case 0:
				barrelXEffect = 1.0f;
				barrelYEffect = 1.0f;
				break;
			case 1:
				barrelXEffect = 0.75f;
				barrelYEffect = 0.4f;
				break;
			case 2:
				barrelXEffect = 0.85f;
				barrelYEffect = 1.0f;
				break;
			case 3:
				barrelXEffect = 1.0f;
				barrelYEffect = 0.85f;
				break;
			}
		}
	}

	float totalAttachXEffect = sightXEffect * barrelXEffect;
	float totalAttachYEffect = sightYEffect * gripEffect * barrelYEffect;

	float scopeModifier = 0.f;

	switch (activescope) {
	case 1:
		scopeModifier = new1xSens;
		break;
	case 2:
		scopeModifier = new25xSens;
		break;
	case 3:
		scopeModifier = new35xSens;
		break;
	}

	// Calculate the sensitivity modifier without attachment effects
	float sensXModifier_SensOnly = ((oldBaseSens * oldRelativeSens * oldMultiplier) / (scopeModifier * newBaseXSens * newMultiplier)) * fovModifier;
	float sensYModifier_SensOnly = ((oldBaseSens * oldRelativeSens * oldMultiplier) / (scopeModifier * newBaseYSens * newMultiplier)) * fovModifier;

	// Set the sensMultiplier_SensOnly (without attachment effects)
	settings.activeState.sensMultiplier_SensOnly = std::vector<float>{ sensXModifier_SensOnly, sensYModifier_SensOnly };

	// Calculate the final sensitivity modifier with attachment effects
	float newSensXModifier = sensXModifier_SensOnly * totalAttachXEffect;
	float newSensYModifier = sensYModifier_SensOnly * totalAttachYEffect;

	return std::vector<float>{ newSensXModifier, newSensYModifier };
}

std::vector<float> calculateSensitivityModifierRust() {
	// Predefined constants
	constexpr float oldBaseSens = 0.509f;
	constexpr float oldADSSens = 1.0f;
	constexpr float oldFov = 90.f;
	constexpr float quadratic_coef = 0.0001787f;
	constexpr float linear_coef = -0.0107192f;

	// Weapons with 1.5x holo modifier
	constexpr std::array<std::string_view, 12> specialHoloWeapons = {
		"High Caliber Revolver", "Python Revolver", "Handmade SMG", "Semi-Automatic Pistol",
		"Thompson", "Custom SMG", "M39", "M4 Shotgun",
		"M92 Pistol", "Prototype 17", "Spas-12 Shotgun"
	};

	float newBaseSens = settings.globalSettings.sensitivity[0];
	float newADSSens = settings.globalSettings.sensitivity[1];
	float newFov = settings.globalSettings.fov;
	float sightEffect = 1.0f;
	float gripEffect = 1.0f;
	float barrelEffect = 1.0f;

	float fovDifference = newFov - oldFov;
	float fovModifier = 1.0f + (quadratic_coef * fovDifference * fovDifference) +
		(linear_coef * fovDifference);

	settings.activeState.fovSensitivityModifier = fovModifier;

	if (settings.activeState.selectedCharacterIndex < settings.characters.size() &&
		!settings.characters[settings.activeState.selectedCharacterIndex].weapondata.empty()) {

		const auto& character = settings.characters[settings.activeState.selectedCharacterIndex];
		int weaponIndex = std::min(character.selectedweapon[0], static_cast<int>(character.weapondata.size()) - 1);
		const auto& weapon = character.weapondata[weaponIndex];

		if (weapon.attachments.size() >= 3) {
			switch (weapon.attachments[0]) {
			case 0:
				sightEffect = 1.0f;
				break;
			case 1:
				sightEffect = 0.8f;
				break;
			case 2:
				sightEffect = (std::find(specialHoloWeapons.begin(),
					specialHoloWeapons.end(),
					weapon.weaponname) != specialHoloWeapons.end()) ? 1.5f : 1.25f;
				break;
			case 3:
				sightEffect = 7.25f;
				break;
			case 4:
				sightEffect = 14.5f;
				break;
			}

			switch (weapon.attachments[1]) {
			case 0:
				gripEffect = 1.0f;
				break;
			case 1:
				gripEffect = 0.9f;
				break;
			}

			switch (weapon.attachments[2]) {
			case 0:
				barrelEffect = 1.0f;
				break;
			case 1:
				barrelEffect = 0.5f;
				break;
			case 2:
				barrelEffect = 1.1f;
				break;
			}
		}
	}

	float totalAttachEffect = sightEffect * gripEffect * barrelEffect;
	float standingModifier = settings.misc.hotkeys.IsActive(HotkeyIndex::CrouchKey) ? 0.5f : 1.0f;

	float movingModifier = (GetAsyncKeyState(0x57) || GetAsyncKeyState(0x41) || GetAsyncKeyState(0x53) || GetAsyncKeyState(0x44)) ? 1.2f : 1.0f;

	// Calculate the sensitivity modifier without attachment effects
	float sensModifier_SensOnly = ((oldBaseSens * oldADSSens) / (newBaseSens * newADSSens) * (standingModifier * movingModifier)) * fovModifier;

	// Set the sensMultiplier_SensOnly (without attachment effects)
	settings.activeState.sensMultiplier_SensOnly = std::vector<float>{ sensModifier_SensOnly, sensModifier_SensOnly };

	// Calculate the final sensitivity modifier with attachment effects
	float sensModifier = sensModifier_SensOnly * totalAttachEffect;

	return std::vector<float>{ sensModifier, sensModifier };
}

std::vector<float> calculateSensitivityModifierOverwatch() {
	settings.activeState.fovSensitivityModifier = 1;
	settings.activeState.sensMultiplier_SensOnly = { 1.f, 1.f };
	return std::vector<float>{ 1.f, 1.f };
}

// Function to parse keybinds and update global struct
void keybindManager() {

	settings.misc.hotkeys.Update();

	//////////////////////////////////////// - Character Detectors

	if (settings.globalSettings.characterDetectors[0]) {
		static bool detectedOperator = false;
		static bool useShootingRangeOffset = false;

		float cropRatioX = 0.f;
		float cropRatioY = 0.0f;
		float cropRatioWidth = 0.f;
		float cropRatioHeight = 0.052f;

		// Define ratios for crop region
		switch (settings.globalSettings.aspect_ratio) {
		case 0:
			cropRatioX = 0.3928f * (useShootingRangeOffset ? 1.139f : 1.f);
			cropRatioWidth = 0.0253f;
			break;
		case 1:
			cropRatioX = 0.358f * (useShootingRangeOffset ? 1.202f : 1.f);
			cropRatioWidth = 0.0325f;
			break;
		case 2:
			cropRatioX = 0.3473f * (useShootingRangeOffset ? 1.224f : 1.f);
			cropRatioWidth = 0.0357f;
			break;
		case 3:
			cropRatioX = 0.3727f * (useShootingRangeOffset ? 1.174f : 1.f);
			cropRatioWidth = 0.03f;
			break;
		case 4:
			cropRatioX = 0.3815f * (useShootingRangeOffset ? 1.158f : 1.f);
			cropRatioWidth = 0.028f;
			break;
		case 5:
			cropRatioX = 0.3857f * (useShootingRangeOffset ? 1.152f : 1.f);
			cropRatioWidth = 0.0263f;
			break;
		case 6:
			cropRatioX = 0.4f * (useShootingRangeOffset ? 1.128f : 1.f);
			cropRatioWidth = 0.023f;
			break;
		case 7:
			cropRatioX = 0.4196f * (useShootingRangeOffset ? 1.097f : 1.f);
			cropRatioWidth = 0.0191f;
			break;
		}

		// Calculate the region of interest (ROI) based on ratios
		int x = static_cast<int>(cropRatioX * globals.capture.desktopWidth);
		int y = static_cast<int>(cropRatioY * globals.capture.desktopHeight);
		int width = static_cast<int>(cropRatioWidth * globals.capture.desktopWidth);
		int height = static_cast<int>(cropRatioHeight * globals.capture.desktopHeight);

		// Ensure the ROI is within the bounds of the desktopMat
		x = std::max(0, x);
		y = std::max(0, y);
		width = std::min(width, globals.capture.desktopWidth - x);
		height = std::min(height, globals.capture.desktopHeight - y);

		cv::Rect roi(x, y, width, height);

		// Extract the region of interest from the desktopMat
		globals.capture.desktopMutex_.lock();
		cv::Mat smallRegion = globals.capture.desktopMat(roi);
		globals.capture.desktopMutex_.unlock();

		detectedOperator = dx.detectOperatorR6(smallRegion);

		if (!detectedOperator) {
			useShootingRangeOffset = !useShootingRangeOffset;
		}
	}

	//////////////////////////////////////// - Weapon Detectors

	if (settings.globalSettings.weaponDetectors[0]) {
		// Define ratios for crop region
		float cropRatioX = 0.0f;
		float cropRatioY = 0.82f;
		float cropRatioWidth = 0.008f;
		float cropRatioHeight = 0.14f;

		switch (settings.globalSettings.aspect_ratio) {
		case 0:
			cropRatioX = 0.8535f;
			break;
		case 1:
			cropRatioX = 0.805f;
			break;
		case 2:
			cropRatioX = 0.793f;
			break;
		case 3:
			cropRatioX = 0.8275f;
			break;
		case 4:
			cropRatioX = 0.838f;
			break;
		case 5:
			cropRatioX = 0.844f;
			break;
		case 6:
			cropRatioX = 0.831f;
			break;
		case 7:
			cropRatioX = 0.765f;
			break;
		}

		// Calculate the region of interest (ROI) based on ratios
		int x = static_cast<int>(cropRatioX * globals.capture.desktopWidth);
		int y = static_cast<int>(cropRatioY * globals.capture.desktopHeight);
		int width = static_cast<int>(cropRatioWidth * globals.capture.desktopWidth);
		int height = static_cast<int>(cropRatioHeight * globals.capture.desktopHeight);

		// Ensure the ROI is within the bounds of the desktopMat
		x = std::max(0, x);
		y = std::max(0, y);
		width = std::min(width, globals.capture.desktopWidth - x);
		height = std::min(height, globals.capture.desktopHeight - y);

		cv::Rect roi(x, y, width, height);

		// Extract the region of interest from the desktopMat
		globals.capture.desktopMutex_.lock();
		cv::Mat smallRegion = globals.capture.desktopMat(roi);
		globals.capture.desktopMutex_.unlock();

		dx.detectWeaponR6(smallRegion, 25, 25);
	}

	if (settings.globalSettings.weaponDetectors[1]) {
		static bool initializeRustDetector = true;

		if (!globals.capture.desktopMat.empty()) {
			// Define ratios for crop region
			float cropRatioX = 0.3475f;
			float cropRatioY = 0.892f;
			float cropRatioWidth = 0.295f;
			float cropRatioHeight = 0.084f;

			// Calculate the region of interest (ROI) based on ratios
			int x = static_cast<int>(cropRatioX * globals.capture.desktopWidth);
			int y = static_cast<int>(cropRatioY * globals.capture.desktopHeight);
			int width = static_cast<int>(cropRatioWidth * globals.capture.desktopWidth);
			int height = static_cast<int>(cropRatioHeight * globals.capture.desktopHeight);

			// Ensure the ROI is within the bounds of the desktopMat
			x = std::max(0, x);
			y = std::max(0, y);
			width = std::min(width, globals.capture.desktopWidth - x);
			height = std::min(height, globals.capture.desktopHeight - y);

			cv::Rect roi(x, y, width, height);

			// Extract the region of interest from the desktopMat
			globals.capture.desktopMutex_.lock();
			cv::Mat smallRegion = globals.capture.desktopMat(roi);
			globals.capture.desktopMutex_.unlock();

			if (!initializeRustDetector) {
				dx.initializeRustDetector(smallRegion);
				initializeRustDetector = true;
			}

			dx.detectWeaponRust(smallRegion);
		}
	}

	//////////////////////////////////////// - Sensitivity Calculators

	switch (settings.globalSettings.sensitivityCalculator) {
	case 0: // Character
		settings.activeState.sensMultiplier = { 1.f, 1.f };
		break;
	case 1: // Siege
		settings.activeState.sensMultiplier = calculateSensitivityModifierR6();
		break;
	case 2: // Rust
		settings.activeState.sensMultiplier = calculateSensitivityModifierOverwatch();
		break;
	case 3: // Overwatch
		settings.activeState.sensMultiplier = calculateSensitivityModifierOverwatch();
		break;
	}
}

void DrawRecoilPattern(const std::vector<std::vector<float>>& recoilData) {
	if (recoilData.empty()) return;

	std::vector<float> sens = settings.activeState.sensMultiplier;

	std::vector<float> x, y;
	float maxX = 0, maxY = 0;
	float currentX = 0, currentY = 0;

	x.push_back(currentX);
	y.push_back(currentY);

	for (const auto& point : recoilData) {
		if (point.size() >= 3) {
			float dx = point[0] * sens[0];
			float dy = point[1] * sens[1];
			float timeAndDistance = point[2];

			// Use time and distance to scale the direction
			currentX += dx * timeAndDistance;
			currentY += dy * timeAndDistance;

			x.push_back(currentX);
			y.push_back(currentY);

			maxX = std::max(maxX, std::abs(currentX));
			maxY = std::max(maxY, std::abs(currentY));
		}
	}

	// Ensure a minimum spread for X-axis
	float minSpread = 7000.0f;
	maxX = maxX * 1.1;
	maxX = std::max(maxX, minSpread / 2.0f);

	// Ensure a minimum spread for Y-axis
	maxY = maxY * 1.1;
	maxY = std::max(maxY, minSpread / 2.0f);

	if (ImPlot::BeginPlot(xorstr_("Recoil Pattern"), ImVec2(-1, -1))) {
		ImPlot::SetupAxes(xorstr_("Horizontal"), xorstr_("Vertical"), ImPlotAxisFlags_RangeFit, ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_Invert);
		ImPlot::SetupAxisLimits(ImAxis_X1, -maxX, maxX, ImGuiCond_Always);
		ImPlot::SetupAxisLimits(ImAxis_Y1, -maxY * 0.1, maxY * 1.1, ImGuiCond_Always);

		// Plot the recoil pattern
		ImPlot::PlotLine(xorstr_("Recoil"), x.data(), y.data(), x.size());

		// Plot points to show each recoil step
		ImPlot::PlotScatter(xorstr_("Steps"), x.data(), y.data(), x.size());

		// Add labels for start and end points
		ImPlot::PlotText(xorstr_("Start"), x[0], y[0], ImVec2(0, -10));
		ImPlot::PlotText(xorstr_("End"), x.back(), y.back(), ImVec2(0, 10));

		ImPlot::EndPlot();
	}
}

void Menu::gui()
{
	bool inverseShutdown = !globals.initshutdown;

	ImGui::SetNextWindowSize(ImVec2(850, 650), ImGuiCond_FirstUseEver);
	#if !_DEBUG
	ImGui::Begin(xorstr_("Focus"), &inverseShutdown, WINDOWFLAGS);
	#else
	ImGui::Begin(xorstr_("Focus DEBUG"), &inverseShutdown, WINDOWFLAGS);
	#endif

	bool initshutdownpopup = false;

	static char newConfigName[256] = "";
	static int selectedMode = 0;
	static int selectedGame = 0;
	bool NewConfigPopup = false;

	static char renameBuffer[256] = "";
	bool RenameConfigPopup = false;

	bool loadConfigPopup = false;
	bool saveConfigPopup = false;
	bool deleteConfigPopup = false;

	static std::vector<std::string> convertedFiles;

	//g.filesystem.unsavedChanges = ut.isEdited(g.characterinfo.jsonData, editor.GetText());

	keybindManager();

	if (ImGui::BeginTabBar(xorstr_("##TabBar")))
	{
		if (ImGui::BeginTabItem(xorstr_("Globals"))) {

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("Global Settings"));

			std::vector<const char*> CharacterDetectors = { xorstr_("Siege Operator Detection") };
			if (multiCombo(xorstr_("Character Detectors"), CharacterDetectors, settings.globalSettings.characterDetectors)) {
				settings.activeState.weaponDataChanged = true;
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Siege Operator Detection - Searches for your Siege Operator"));

			std::vector<const char*> WeaponDetectors = { xorstr_("Siege Weapon Detection"), xorstr_("Rust Weapon Detection") };
			if (multiCombo(xorstr_("Weapon Detectors"), WeaponDetectors, settings.globalSettings.weaponDetectors)) {
				settings.activeState.weaponDataChanged = true;
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Siege Weapon Detection - Swaps between the primary and secondary in Siege\nRust Weapon Detection - Sets your weapon based on the selection in your Rust hotbar"));

			std::vector<const char*> SensitivityCalculator = { xorstr_("Generic"), xorstr_("Siege"), xorstr_("Rust"), xorstr_("Overwatch") };
			if (comboBoxGen(xorstr_("Sensitivity Calculator"), &settings.globalSettings.sensitivityCalculator, SensitivityCalculator.data(), SensitivityCalculator.size())) {
				globals.filesystem.unsavedChanges = true;
				settings.activeState.weaponDataChanged = true;
			}

			if (ImGui::Checkbox(xorstr_("Potato Mode"), &settings.globalSettings.potato)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Significantly reduces CPU usage but causes more first-shot recoil"));

			const char* Aspect_Ratios[] = { xorstr_("16:9"), xorstr_("4:3"), xorstr_("5:4"), xorstr_("3:2"), xorstr_("16:10"), xorstr_("5:3"), xorstr_("19:10"), xorstr_("21:9") };
			if (comboBoxGen(xorstr_("Aspect Ratio"), &settings.globalSettings.aspect_ratio, Aspect_Ratios, 8)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Settings -> Display -> Aspect Ratio"));

			ImGui::Spacing();

			if (ImGui::SliderFloat(xorstr_("FOV"), &settings.globalSettings.fov, 60.0f, 103.0f, xorstr_("%.0f"))) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Settings -> Display -> Field of View"));

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("Sensitivity"));

			switch (settings.globalSettings.sensitivityCalculator) {
			case 0: // Generic
				// No sensitivity settings
				break;
			case 1: // Siege
				if (ImGui::SliderFloat(xorstr_("X Base Sensitivity"), &settings.globalSettings.sensitivity[0], 0.0f, 100.0f, xorstr_("%.0f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("Settings -> Controls -> Mouse Sensitivity Horizontal"));
				if (ImGui::SliderFloat(xorstr_("Y Base Sensitivity"), &settings.globalSettings.sensitivity[1], 0.0f, 100.0f, xorstr_("%.0f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("Settings -> Controls -> Mouse Sensitivity Vertical"));
				if (ImGui::SliderFloat(xorstr_("1x Sensitivity"), &settings.globalSettings.sensitivity[2], 0.0f, 200.0f, xorstr_("%.0f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("Settings -> Controls -> Mouse ADS Sensitivity -> 1.0x Magnification"));
				if (ImGui::SliderFloat(xorstr_("2.5x Sensitivity"), &settings.globalSettings.sensitivity[3], 0.0f, 200.0f, xorstr_("%.0f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("Settings -> Controls -> Mouse ADS Sensitivity -> 2.5x Magnification"));
				if (ImGui::SliderFloat(xorstr_("3.5x Sensitivity"), &settings.globalSettings.sensitivity[4], 0.0f, 200.0f, xorstr_("%.0f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("Settings -> Controls -> Mouse ADS Sensitivity -> 3.5x Magnification"));
				if (ImGui::SliderFloat(xorstr_("Sensitivity Multiplier"), &settings.globalSettings.sensitivity[5], 0.0f, 1.0f, xorstr_("%.3f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				break;
			case 2:
				if (ImGui::SliderFloat(xorstr_("Sensitivity"), &settings.globalSettings.sensitivity[0], 0.0f, 10.0f, xorstr_("%.3f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				if (ImGui::SliderFloat(xorstr_("Aiming Sensitivity"), &settings.globalSettings.sensitivity[1], 0.0f, 10.0f, xorstr_("%.3f"))) {
					globals.filesystem.unsavedChanges = true;
				}
				break;
			case 3:
				// No sensitivity settings
				break;
			}

			ImGui::Text(xorstr_("X Sensitivity Modifier: %f"), settings.activeState.sensMultiplier[0]);
			ImGui::Text(xorstr_("Y Sensitivity Modifier: %f"), settings.activeState.sensMultiplier[1]);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(xorstr_("Character"))) {

			ImGui::Columns(2);
			ImGui::BeginChild(xorstr_("Left Column"));

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("Character Selectors"));

			if (settings.characters.size() > 0) {

				if (comboBoxChar(xorstr_("Character"), settings.activeState.selectedCharacterIndex, settings.characters)) {
					settings.activeState.weaponDataChanged = true;
				}

				ImGui::Spacing();
				ImGui::Spacing();

				if (settings.activeState.weaponOffOverride) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
					ImGui::SeparatorText(xorstr_("Primary"));
					ImGui::PopStyleColor();
				}
				else if (settings.activeState.isPrimaryActive) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
					ImGui::SeparatorText(xorstr_("Primary"));
					ImGui::PopStyleColor();
				}
				else {
					ImGui::SeparatorText(xorstr_("Primary"));
				}

				if (comboBoxWep(xorstr_("Primary"), settings.activeState.selectedCharacterIndex, settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0], settings.characters)) {
					settings.activeState.weaponDataChanged = true;
					globals.filesystem.unsavedChanges = true;
				}
				if (ImGui::Checkbox(xorstr_("Primary Rapid Fire"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].rapidfire)) {
					settings.activeState.weaponDataChanged = true;
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("Hold Left+Right mouse to automatically shoot"));

				switch (settings.globalSettings.sensitivityCalculator) {
				case 0: // Generic
					// No attachments
					break;
				case 1: // Siege
					{
						const char* Sights[] = { xorstr_("1x"), xorstr_("2.5x"), xorstr_("3.5x") };
						const char* Grips[] = { xorstr_("Horizontal/Angled/None"), xorstr_("Vertical") };
						const char* Barrels[] = { xorstr_("Supressor/Extended/None"), xorstr_("Muzzle Break (Semi-Auto Only)"), xorstr_("Compensator"), xorstr_("Flash Hider") };
						if (comboBoxGen(xorstr_("Primary Sight"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].attachments[0], Sights, 3)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Grip"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].attachments[1], Grips, 2)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Barrel"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].attachments[2], Barrels, 4)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
					}
					break;
				case 2: // Rust
					{
						const char* Sights[] = { xorstr_("None"), xorstr_("Handmade"), xorstr_("Holosight"), xorstr_("8x Scope"), xorstr_("16x Scope") };
						const char* Grips[] = { xorstr_("None"), xorstr_("Gas Compression Overdrive") };
						const char* Barrels[] = { xorstr_("None/Suppressor"), xorstr_("Muzzle Break"), xorstr_("Muzzle Boost") };
						if (comboBoxGen(xorstr_("Primary Sight"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].attachments[0], Sights, 3)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Grip"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].attachments[1], Grips, 2)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Barrel"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].attachments[2], Barrels, 4)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
					}
					break;
				case 3: // Overwatch
					// No attachments
					break;
				}

				ImGui::Spacing();
				ImGui::Spacing();

				if (settings.activeState.weaponOffOverride) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
					ImGui::SeparatorText(xorstr_("Secondary"));
					ImGui::PopStyleColor();
				}
				else if (!settings.activeState.isPrimaryActive) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
					ImGui::SeparatorText(xorstr_("Secondary"));
					ImGui::PopStyleColor();
				}
				else {
					ImGui::SeparatorText(xorstr_("Secondary"));
				}

				if (comboBoxWep(xorstr_("Secondary"), settings.activeState.selectedCharacterIndex, settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1], settings.characters)) {
					settings.activeState.weaponDataChanged = true;
					globals.filesystem.unsavedChanges = true;
				}
				if (ImGui::Checkbox(xorstr_("Secondary Rapid Fire"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]].rapidfire)) {
					settings.activeState.weaponDataChanged = true;
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("Hold Left+Right mouse to automatically shoot"));

				switch (settings.globalSettings.sensitivityCalculator) {
				case 0: // Generic
					// No attachments
					break;
				case 1: // Siege
					{
						const char* Sights[] = { xorstr_("1x"), xorstr_("2.5x"), xorstr_("3.5x") };
						const char* Grips[] = { xorstr_("Horizontal/Angled/None"), xorstr_("Vertical") };
						const char* Barrels[] = { xorstr_("Supressor/Extended/None"), xorstr_("Muzzle Break (Semi-Auto Only)"), xorstr_("Compensator"), xorstr_("Flash Hider") };
						if (comboBoxGen(xorstr_("Primary Sight"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]].attachments[0], Sights, 3)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Grip"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]].attachments[1], Grips, 2)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Barrel"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]].attachments[2], Barrels, 4)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
					}
					break;
				case 2: // Rust
					{
						const char* Sights[] = { xorstr_("None"), xorstr_("Handmade"), xorstr_("Holosight"), xorstr_("8x Scope"), xorstr_("16x Scope") };
						const char* Grips[] = { xorstr_("None"), xorstr_("Gas Compression Overdrive") };
						const char* Barrels[] = { xorstr_("None/Suppressor"), xorstr_("Muzzle Break"), xorstr_("Muzzle Boost") };
						if (comboBoxGen(xorstr_("Primary Sight"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]].attachments[0], Sights, 3)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Grip"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]].attachments[1], Grips, 2)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Barrel"), &settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]].attachments[2], Barrels, 4)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
					}
					break;
				case 3: // Overwatch
					// No attachments
					break;
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::SeparatorText(xorstr_("Character Settings"));

				if (settings.globalSettings.weaponDetectors[0]) {
					std::vector<const char*> CharacterOptions = { xorstr_("Siege Gadget Detection Override") };
					if (multiCombo(xorstr_("Options"), CharacterOptions, settings.characters[settings.activeState.selectedCharacterIndex].options)) {
						settings.activeState.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("Siege Gadget Detection Override - Prevents gadgets from disabling weapon detection in Siege"));
				}

				ImGui::Text(xorstr_("X Sensitivity Modifier: %f"), settings.activeState.sensMultiplier[0]);
				ImGui::Text(xorstr_("Y Sensitivity Modifier: %f"), settings.activeState.sensMultiplier[1]);
			}
			else {
				ImGui::Text(xorstr_("Please load a weapons file"));
			}

			ImGui::EndChild();
			ImGui::NextColumn();
			ImGui::BeginChild(xorstr_("Right Column"));

			if (settings.characters.size() > 0 && settings.activeState.selectedCharacterIndex < settings.characters.size()) {
				const auto& activeWeapon = settings.activeState.isPrimaryActive ?
					settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]] :
					settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1]];

				DrawRecoilPattern(activeWeapon.values);
			}
			else {
				ImGui::Text(xorstr_("No weapon selected or data available."));
			}

			ImGui::EndChild();
			ImGui::Columns(1);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(xorstr_("Aim"))) {
			std::vector<const char*> AimbotType = { xorstr_("Colour"), xorstr_("AI") };
			if (ImGui::Combo(xorstr_("Aimbot Type"), &settings.aimbotData.type, AimbotType.data(), (int)AimbotType.size())) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Colour - Aimbot using outline detection\nAI - Aimbot using AI"));

			if (settings.aimbotData.type == 0) {
				if (ImGui::Checkbox(xorstr_("Colour Aimbot"), &settings.aimbotData.enabled)) {
					globals.filesystem.unsavedChanges = true;
				}
			}
			else if (settings.aimbotData.type == 1) {
				if (!settings.aimbotData.enabled) {
					std::vector<const char*> providers = { xorstr_("CPU"), xorstr_("CUDA"), xorstr_("TensorRT") };
					if (ImGui::Combo(xorstr_("Provider"), &settings.aimbotData.aiAimbotSettings.provider, providers.data(), (int)providers.size())) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("CPU is recommended for most systems (Unless you have a 4080/4090)"));
				}

				if (ImGui::Checkbox(xorstr_("AI Aim Assist"), &settings.aimbotData.enabled)) {
					globals.filesystem.unsavedChanges = true;
				}
			}

			if (settings.aimbotData.enabled) {
				std::vector<const char*> pidPresets = { xorstr_("Slow"), xorstr_("Legit"), xorstr_("Aggressive"), xorstr_("Custom") };

				if (ImGui::Combo(xorstr_("Aimbot Preset"), &settings.aimbotData.pidSettings.pidPreset, pidPresets.data(), (int)pidPresets.size())) {
					globals.filesystem.unsavedChanges = true;

					if (settings.aimbotData.pidSettings.pidPreset == 0) {
						settings.aimbotData.pidSettings.proportional = 0.005f;
						settings.aimbotData.pidSettings.integral = 0.05f;
						settings.aimbotData.pidSettings.derivative = 0.008f;
						settings.aimbotData.pidSettings.rampUpTime = 0.0f;
						settings.activeState.pidDataChanged = true;
					}
					else if (settings.aimbotData.pidSettings.pidPreset == 1) {
						settings.aimbotData.pidSettings.proportional = 0.02f;
						settings.aimbotData.pidSettings.integral = 0.2f;
						settings.aimbotData.pidSettings.derivative = 0.008f;
						settings.aimbotData.pidSettings.rampUpTime = 1.f;
						settings.activeState.pidDataChanged = true;
					}
					else if (settings.aimbotData.pidSettings.pidPreset == 2) {
						settings.aimbotData.pidSettings.proportional = 0.035f;
						settings.aimbotData.pidSettings.integral = 0.8f;
						settings.aimbotData.pidSettings.derivative = 0.008f;
						settings.aimbotData.pidSettings.rampUpTime = 1.f;
						settings.activeState.pidDataChanged = true;
					}
				}

				if (settings.aimbotData.pidSettings.pidPreset == 3) {
					if (ImGui::InputFloat(xorstr_("Proportional"), &settings.aimbotData.pidSettings.proportional, 0.0f, 0.0f)) {
						globals.filesystem.unsavedChanges = true;
						settings.activeState.pidDataChanged = true;
					}
					tooltip(xorstr_("How quickly the aimbot moves"));
					if (ImGui::InputFloat(xorstr_("Integral"), &settings.aimbotData.pidSettings.integral, 0.0f, 0.0f)) {
						globals.filesystem.unsavedChanges = true;
						settings.activeState.pidDataChanged = true;
					}
					tooltip(xorstr_("How quickly the aimbot changes direction"));
					if (ImGui::InputFloat(xorstr_("Derivative"), &settings.aimbotData.pidSettings.derivative, 0.0f, 0.0f)) {
						globals.filesystem.unsavedChanges = true;
						settings.activeState.pidDataChanged = true;
					}
					tooltip(xorstr_("How far ahead in time the aimbot looks"));
					if (ImGui::InputFloat(xorstr_("Integral Ramp Up Time"), &settings.aimbotData.pidSettings.rampUpTime, 0.0f, 0.0f)) {
						globals.filesystem.unsavedChanges = true;
						settings.activeState.pidDataChanged = true;
					}
					tooltip(xorstr_("The amount of time it takes to reach the full PID value (Starts from 0)"));
				}

				if (ImGui::SliderInt(xorstr_("Max Distance per Tick"), &settings.aimbotData.maxDistance, 1, 100)) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("The farthest a single aimbot movement can travel"));

				if (ImGui::SliderInt(xorstr_("Aimbot FOV"), &settings.aimbotData.aimFov, 0, 50, xorstr_("%d%%"))) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("The percentage of the screen that the aimbot can see (Within the centre 50% of your screen)"));

				if (ImGui::SliderInt(xorstr_("Aimbot Vertical Correction Modifier"), &settings.aimbotData.verticalCorrection, 1, 100, xorstr_("%d%%"))) {
					globals.filesystem.unsavedChanges = true;
				}
				tooltip(xorstr_("How much of the vertical aimbot correction is actually applied"));

				if (settings.aimbotData.type == 0) {
					std::vector<const char*> colourAimbotPreset = { xorstr_("Default"), xorstr_("Custom") };
					if (ImGui::Combo(xorstr_("Colour Aimbot Detection Preset"), &settings.aimbotData.colourAimbotSettings.detectionPreset, colourAimbotPreset.data(), (int)colourAimbotPreset.size())) {
						globals.filesystem.unsavedChanges = true;

						if (settings.aimbotData.colourAimbotSettings.detectionPreset == 0) {
							settings.aimbotData.colourAimbotSettings.maxTrackAge = 3;
							settings.aimbotData.colourAimbotSettings.trackSmoothingFactor = 0;
							settings.aimbotData.colourAimbotSettings.trackConfidenceRate = 20;
							settings.aimbotData.colourAimbotSettings.maxClusterDistance = 10;
							settings.aimbotData.colourAimbotSettings.maxClusterDensityDifferential = 80;
							settings.aimbotData.colourAimbotSettings.minDensity = 10;
							settings.aimbotData.colourAimbotSettings.minArea = 5;
							settings.aimbotData.colourAimbotSettings.aimHeight = -5;
						}
					}

					if (settings.aimbotData.colourAimbotSettings.detectionPreset == 1) {
						if (ImGui::InputInt(xorstr_("Max Tracking Age"), &settings.aimbotData.colourAimbotSettings.maxTrackAge, 0, 0)) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("How long a point is preserved after losing tracking (Measured in frames)"));
						if (ImGui::SliderInt(xorstr_("Tracking Smoothing Factor"), &settings.aimbotData.colourAimbotSettings.trackSmoothingFactor, 0, 100, xorstr_("%d%%"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("How strongly past positions are favoured in tracking"));
						if (ImGui::SliderInt(xorstr_("Tracking Confidence Rate"), &settings.aimbotData.colourAimbotSettings.trackConfidenceRate, 0, 100, xorstr_("%d%%"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("How fast confidence in a detection is gained or lost"));
						if (ImGui::SliderInt(xorstr_("Max Cluster Distance"), &settings.aimbotData.colourAimbotSettings.trackConfidenceRate, 0, 100, xorstr_("%d%%"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("How far apart points can be, to be classified as a single cluster"));
						if (ImGui::SliderInt(xorstr_("Max Cluster Density Differential"), &settings.aimbotData.colourAimbotSettings.maxClusterDensityDifferential, 0, 100, xorstr_("%d%%"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("How similar the density of clusters must be, in order to be joined"));
						if (ImGui::SliderInt(xorstr_("Minimum Cluster Density"), &settings.aimbotData.colourAimbotSettings.minDensity, 0, 100, xorstr_("%d%%"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("How dense a cluster must be to be considered valid"));
						if (ImGui::InputInt(xorstr_("Minimum Cluster Area"), &settings.aimbotData.colourAimbotSettings.minArea, 0, 0)) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("How large a cluster must be to be considered valid"));
						if (ImGui::SliderInt(xorstr_("Target Height Correction"), &settings.aimbotData.colourAimbotSettings.aimHeight, -100, 100, xorstr_("%d%%"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Vertical correction above or below the original aim point"));
						if (ImGui::Checkbox(xorstr_("Enable Debug View"), &settings.aimbotData.colourAimbotSettings.debugView)) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Allows you to visualise the detection pipeline but severely impacts performance"));
					}
				}
				else if (settings.aimbotData.type == 1) {
					if (ImGui::SliderInt(xorstr_("Minimum Required Confidence"), &settings.aimbotData.aiAimbotSettings.confidence, 1, 100, xorstr_("%d%%"))) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("Higher confidences are less likely to falsely detect an enemy, but also results in less detections overall"));

					if (ImGui::Checkbox(xorstr_("Force Hitbox Selection"), &settings.aimbotData.aiAimbotSettings.forceHitbox)) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("Will force the aimbot to target the selected hitbox, if the hitbox isn't detected, the aimbot will not move"));

					std::vector<const char*> AimbotHitbox = { xorstr_("Body"), xorstr_("Head"), xorstr_("Closest") };
					if (ImGui::Combo(xorstr_("Hitbox Priority"), &settings.aimbotData.aiAimbotSettings.hitbox, AimbotHitbox.data(), (int)AimbotHitbox.size())) {
						globals.filesystem.unsavedChanges = true;
					}

					float ms = globals.inferenceTimeMs.load();
					ImVec4 timeColor;
					if (ms <= 10.0f)
						timeColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);  // Green for fast
					else if (ms <= 30.0f)
						timeColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);  // Yellow for medium
					else if (ms <= 50.0f)
						timeColor = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);  // Orange for slow
					else
						timeColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red for very slow

					ImGui::PushStyleColor(ImGuiCol_Text, timeColor);
					ImGui::Text("Inference: %.1f ms", ms);
					ImGui::PopStyleColor();
				}
			}

			if (ImGui::Checkbox(xorstr_("Limit Detector FPS"), &settings.aimbotData.limitDetectorFps)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Locks the aimbot and triggerbot detector to 66 FPS, this reduces CPU usage but will reduce accuracy"));

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("TriggerBot Settings"));

			const char* detectionMethods[] = { xorstr_("Color Change"), xorstr_("Motion"), xorstr_("Both") };
			if (ImGui::Combo(xorstr_("Detection Method"),
				&settings.aimbotData.triggerSettings.detectionMethod,
				detectionMethods, 3)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Color Change: Detect average color changes\nMotion: Detect pixel movement\nBoth: Use both methods"));

			if (ImGui::SliderFloat(xorstr_("Detection Sensitivity"), &settings.aimbotData.triggerSettings.sensitivity, 1.0f, 50.0f, xorstr_("%.1f"))) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Lower values make the triggerbot more sensitive to changes"));

			if (ImGui::SliderInt(xorstr_("Detection Radius"), &settings.aimbotData.triggerSettings.radius, 1, 20)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Size of the area around crosshair to check for changes"));

			if (ImGui::Checkbox(xorstr_("Show Debug View"), &settings.aimbotData.triggerSettings.showDebug)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Visualize the detection area and change values"));

			if (ImGui::SliderInt(xorstr_("Triggerbot Sleep"), &settings.aimbotData.triggerSettings.sleepTime, 0, 2000, xorstr_("%dms%"))) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("How long the Triggerbot will wait between shots"));

			if (ImGui::SliderInt(xorstr_("Triggerbot Burst Duration"), &settings.aimbotData.triggerSettings.burstDuration, 0, 500, xorstr_("%dms%"))) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("How long to hold mouse button down (0 = disabled)"));

			if (ImGui::SliderInt(xorstr_("Triggerbot Fire Delay"), &settings.aimbotData.triggerSettings.delay, 0, 500, xorstr_("%dms%"))) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("How long to wait before firing after target detection (0 = disabled)"));

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(xorstr_("Misc"))) {
			std::vector<const char*> keysSettings = { xorstr_("Require Both"), xorstr_("Just Left Mouse"), xorstr_("Just Right Mouse"), xorstr_("Custom Key") };

			ImGui::SeparatorText(xorstr_("Quick-peek"));
			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Auto Quick-Peek"), HotkeyIndex::AutoQuickPeek)) {
				globals.filesystem.unsavedChanges = true;
			}

			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Auto Hashom Peek"), HotkeyIndex::AutoHashomPeek)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Prone peeking\nMake sure the Prone Key is bound"));

			if (ImGui::SliderInt(xorstr_("Auto Quick-Peek Delay"), &settings.misc.quickPeekDelay, 0, 200, xorstr_("%dms%"))) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("How long it takes to reset the lean while quick-peeking"));

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("In-Game Binds"));
			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Prone Key"), HotkeyIndex::ProneKey)) {
				globals.filesystem.unsavedChanges = true;
			}

			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Left Lean Key"), HotkeyIndex::LeanLeftKey)) {
				globals.filesystem.unsavedChanges = true;
			}

			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Right Lean Key"), HotkeyIndex::LeanRightKey)) {
				globals.filesystem.unsavedChanges = true;
			}

			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Crouch Key"), HotkeyIndex::CrouchKey)) {
				globals.filesystem.unsavedChanges = true;
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("Aimbot Controls"));
			if (ImGui::Combo(xorstr_("Aimbot Mouse Buttons"), &settings.misc.aimKeyMode, keysSettings.data(), (int)keysSettings.size())) {
				globals.filesystem.unsavedChanges = true;
			}
			if (settings.misc.aimKeyMode == 3) {
				if (settings.misc.hotkeys.RenderHotkey(xorstr_("Custom Aimbot Key"), HotkeyIndex::AimKey)) {
					globals.filesystem.unsavedChanges = true;
				}
			}

			if (ImGui::Combo(xorstr_("Recoil Mouse Buttons"), &settings.misc.recoilKeyMode, keysSettings.data(), (int)keysSettings.size())) {
				globals.filesystem.unsavedChanges = true;
			}
			if (settings.misc.recoilKeyMode == 3) {
				if (settings.misc.hotkeys.RenderHotkey(xorstr_("Custom Recoil Key"), HotkeyIndex::RecoilKey)) {
					globals.filesystem.unsavedChanges = true;
				}
			}

			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Triggerbot Key"), HotkeyIndex::TriggerKey)) {
				globals.filesystem.unsavedChanges = true;
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("Misc"));
			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Fake Spinbot"), HotkeyIndex::FakeSpinBot)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Hold to spin really fast"));

			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Hide UI Key"), HotkeyIndex::HideUiKey)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Hide the focus UI"));

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("Overlay"));
			
			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Overlay"), HotkeyIndex::OverlayKey)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Show the FOCUS overlay (Required for magnifier)"));

			if (ImGui::Checkbox(xorstr_("Draw status to overlay"), &settings.misc.overlay.showInfo)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Shows many different settings and states in the overlay"));

			// Add hotkey for magnifier
			if (settings.misc.hotkeys.RenderHotkey(xorstr_("Magnifier Key"), HotkeyIndex::MagnifierKey)) {
				globals.filesystem.unsavedChanges = true;
			}
			tooltip(xorstr_("Zoom in on your crosshair"));

			// Add slider for magnifier zoom
			if (ImGui::SliderFloat(xorstr_("Zoom Level"), &settings.misc.overlay.magnifierZoom, 1.0f, 10.0f, "%.1fx")) {
				globals.filesystem.unsavedChanges = true;
			}

			// Add slider for magnifier size
			if (ImGui::SliderInt(xorstr_("Size"), &settings.misc.overlay.magnifierSize, 100, 800, "%d px")) {
				globals.filesystem.unsavedChanges = true;
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(xorstr_("Config Editor"))) {
			static bool editingPattern = false;

			ImGui::Columns(2, xorstr_("ConfigEditorColumns"), false);

			// Left column: All controls and editors
			ImGui::BeginChild(xorstr_("ControlsAndEditors"));

			if (!editingPattern) {
				ImGui::SeparatorText(xorstr_("Characters"));

				// Character list
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
				ImGui::BeginChild(xorstr_("CharacterList"), ImVec2(0, 100), true);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
				for (int i = 0; i < settings.characters.size(); i++) {
					const bool is_selected = (settings.activeState.selectedCharacterIndex == i);
					if (ImGui::Selectable(settings.characters[i].charactername.c_str(), is_selected)) {
						settings.activeState.selectedCharacterIndex = i;
						settings.activeState.isPrimaryActive = true;
						settings.activeState.weaponDataChanged = true;
					}

					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::PopStyleVar(); // ItemSpacing

				ImGui::EndChild();
				ImGui::PopStyleColor(); // Border color
				ImGui::PopStyleVar(); // ChildBorderSize

				if (ImGui::Button(xorstr_("Add Character"))) {
					settings.characters.push_back(characterData());
					settings.characters.back().charactername = xorstr_("New Character");
					// Initialize the new character with default weapons
					settings.characters.back().weapondata.push_back(weaponData());
					settings.characters.back().weapondata.push_back(weaponData());
					settings.characters.back().weapondata[0].weaponname = xorstr_("Primary Weapon");
					settings.characters.back().weapondata[1].weaponname = xorstr_("Secondary Weapon");
					settings.characters.back().selectedweapon = { 0, 1 };
					// Add default recoil pattern
					std::vector<std::vector<float>> defaultPattern = {
						{0.0f, 5.0f, 1000.0f},
						{0.0f, 5.0f, 1000.0f},
						{0.0f, 5.0f, 1000.0f}
					};
					settings.characters.back().weapondata[0].values = defaultPattern;
					settings.characters.back().weapondata[1].values = defaultPattern;
					// Auto-select the new character
					settings.activeState.selectedCharacterIndex = settings.characters.size() - 1;
					settings.activeState.isPrimaryActive = true;
					settings.activeState.weaponDataChanged = true;
					globals.filesystem.unsavedChanges = true;
				}

				ImGui::SameLine();

				if (ImGui::Button(xorstr_("Remove Character")) && settings.activeState.selectedCharacterIndex < settings.characters.size()) {
					if (settings.characters.size() > 1) { // Ensure there's always at least one character
						settings.characters.erase(settings.characters.begin() + settings.activeState.selectedCharacterIndex);
						settings.activeState.selectedCharacterIndex = std::max(0, static_cast<int>(settings.characters.size()) - 1);
						settings.activeState.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}
					else {
						// Optionally, show a message to the user
						ImGui::OpenPopup(xorstr_("Cannot Remove Character"));
					}
				}

				if (ImGui::BeginPopupModal(xorstr_("Cannot Remove Character"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text(xorstr_("Cannot remove the last character."));
					if (ImGui::Button(xorstr_("OK"), ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}

				if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
					char characterNameBuffer[256];
					strncpy(characterNameBuffer, settings.characters[settings.activeState.selectedCharacterIndex].charactername.c_str(), sizeof(characterNameBuffer));
					characterNameBuffer[sizeof(characterNameBuffer) - 1] = '\0';

					if (ImGui::InputText(xorstr_("Character Name"), characterNameBuffer, sizeof(characterNameBuffer))) {
						if (strlen(characterNameBuffer) > 0) {
							settings.characters[settings.activeState.selectedCharacterIndex].charactername = characterNameBuffer;
							globals.filesystem.unsavedChanges = true;
						}
					}
				}

				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::SeparatorText(xorstr_("Character Options"));

				// Character-specific options based on game type
				if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
					std::vector<const char*> CharacterOptions;

					// Build list of available options based on gameType
					if (settings.globalSettings.sensitivityCalculator == 1) { // Siege
						CharacterOptions = { xorstr_("Siege Gadget Detection Override") };
					}
					else if (settings.globalSettings.sensitivityCalculator == 2) { // Rust
						CharacterOptions = { xorstr_("Rust-specific Option") };
					}

					if (!CharacterOptions.empty()) {
						if (multiCombo(xorstr_("Character Options"), CharacterOptions, settings.characters[settings.activeState.selectedCharacterIndex].options)) {
							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
					}
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::SeparatorText(xorstr_("Weapons"));

				// Weapon list
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
				ImGui::BeginChild(xorstr_("WeaponList"), ImVec2(0, 100), true);

				ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

				// Always use the current character's weapons
				std::vector<weaponData>* currentWeaponData = nullptr;
				if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
					currentWeaponData = &settings.characters[settings.activeState.selectedCharacterIndex].weapondata;
				}

				if (currentWeaponData) {
					for (int i = 0; i < currentWeaponData->size(); i++) {
						const bool is_selected = (settings.activeState.isPrimaryActive ?
							settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] == i :
							settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1] == i);

						const char* displayName = (*currentWeaponData)[i].weaponname.empty() ?
							xorstr_("Unnamed Weapon") :
							(*currentWeaponData)[i].weaponname.c_str();

						if (ImGui::Selectable(displayName, is_selected)) {
							if (settings.activeState.isPrimaryActive)
								settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] = i;
							else
								settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1] = i;

							settings.activeState.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						if (is_selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
				}
				ImGui::PopStyleVar(); // ItemSpacing

				ImGui::EndChild();
				ImGui::PopStyleColor(); // Border color
				ImGui::PopStyleVar(); // ChildBorderSize

				if (ImGui::Button(xorstr_("Add Weapon"))) {
					if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
						settings.characters[settings.activeState.selectedCharacterIndex].weapondata.push_back(weaponData());
						settings.characters[settings.activeState.selectedCharacterIndex].weapondata.back().weaponname = xorstr_("New Weapon");

						// Initialize attachments based on game type
						settings.characters[settings.activeState.selectedCharacterIndex].weapondata.back().attachments.resize(3, 0);

						// Add default recoil pattern
						std::vector<std::vector<float>> defaultPattern = {
							{0.0f, 5.0f, 1000.0f},
							{0.0f, 5.0f, 1000.0f},
							{0.0f, 5.0f, 1000.0f}
						};
						settings.characters[settings.activeState.selectedCharacterIndex].weapondata.back().values = defaultPattern;

						// Auto-select the new weapon
						settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[settings.activeState.isPrimaryActive ? 0 : 1] =
							settings.characters[settings.activeState.selectedCharacterIndex].weapondata.size() - 1;

						settings.activeState.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}
				}

				ImGui::SameLine();

				if (ImGui::Button(xorstr_("Remove Weapon"))) {
					std::vector<weaponData>* currentWeaponData = nullptr;
					if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
						currentWeaponData = &settings.characters[settings.activeState.selectedCharacterIndex].weapondata;
					}

					if (currentWeaponData && currentWeaponData->size() > 1) { // Ensure there's always at least one weapon
						int& selectedPrimaryIndex = settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0];
						int& selectedSecondaryIndex = settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1];
						int indexToRemove = settings.activeState.isPrimaryActive ? selectedPrimaryIndex : selectedSecondaryIndex;

						// Remove the weapon
						currentWeaponData->erase(currentWeaponData->begin() + indexToRemove);

						// After deletion, validate and adjust both primary and secondary indices
						size_t maxWeaponIndex = currentWeaponData->size() - 1;

						// Adjust indices that were above the deleted index
						if (selectedPrimaryIndex > indexToRemove) {
							selectedPrimaryIndex--;
						}
						if (selectedSecondaryIndex > indexToRemove) {
							selectedSecondaryIndex--;
						}

						// Validate both indices are within bounds
						if (selectedPrimaryIndex > maxWeaponIndex) {
							selectedPrimaryIndex = maxWeaponIndex;
						}
						if (selectedSecondaryIndex > maxWeaponIndex) {
							selectedSecondaryIndex = maxWeaponIndex;
						}

						settings.activeState.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}
					else {
						ImGui::OpenPopup(xorstr_("Cannot Remove Weapon"));
					}
				}

				if (ImGui::BeginPopupModal(xorstr_("Cannot Remove Weapon"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text(xorstr_("Cannot remove the last weapon."));
					if (ImGui::Button(xorstr_("OK"), ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}

				if (currentWeaponData && !currentWeaponData->empty()) {
					int& selectedWeaponIndex = settings.activeState.isPrimaryActive ?
						settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] :
						settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1];

					selectedWeaponIndex = std::min(selectedWeaponIndex, static_cast<int>(currentWeaponData->size()) - 1);

					auto& weapon = (*currentWeaponData)[selectedWeaponIndex];

					char weaponNameBuffer[256];
					strncpy(weaponNameBuffer, weapon.weaponname.c_str(), sizeof(weaponNameBuffer));
					weaponNameBuffer[sizeof(weaponNameBuffer) - 1] = '\0';

					if (ImGui::InputText(xorstr_("Weapon Name"), weaponNameBuffer, sizeof(weaponNameBuffer))) {
						if (strlen(weaponNameBuffer) > 0) {
							weapon.weaponname = weaponNameBuffer;
							globals.filesystem.unsavedChanges = true;
						}
					}

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SeparatorText(xorstr_("Weapon Data"));

					if (ImGui::Checkbox(xorstr_("Rapid Fire"), &weapon.rapidfire)) {
						globals.filesystem.unsavedChanges = true;
						settings.activeState.weaponDataChanged = true;
					}

					// Attachments based on game type
					switch (settings.globalSettings.sensitivityCalculator) {
					case 0: // Character
						{
							break;
						}
					case 1: // Siege
						{
							ImGui::Text(xorstr_("Attachments"));
							const char* Sights[] = { xorstr_("1x"), xorstr_("2.5x"), xorstr_("3.5x") };
							const char* Grips[] = { xorstr_("Horizontal/Angled/None"), xorstr_("Vertical") };
							const char* Barrels[] = { xorstr_("Suppressor/Extended/None"), xorstr_("Muzzle Break (Semi-Auto Only)"), xorstr_("Compensator"), xorstr_("Flash Hider") };

							// Ensure the attachment vector has the correct size
							if (weapon.attachments.size() < 3)
								weapon.attachments.resize(3, 0);

							if (ImGui::Combo(xorstr_("Sight"), &weapon.attachments[0], Sights, IM_ARRAYSIZE(Sights))) {
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Grip"), &weapon.attachments[1], Grips, IM_ARRAYSIZE(Grips))) {
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Barrel"), &weapon.attachments[2], Barrels, IM_ARRAYSIZE(Barrels))) {
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}
							break;
						}
					case 2: // Rust
						{
							ImGui::Text(xorstr_("Attachments"));
							const char* RustSights[] = { xorstr_("None"), xorstr_("Handmade"), xorstr_("Holosight"), xorstr_("8x Scope"), xorstr_("16x Scope") };
							const char* RustGrips[] = { xorstr_("None"), xorstr_("Gas Compression Overdrive") };
							const char* RustBarrels[] = { xorstr_("None/Suppressor"), xorstr_("Muzzle Break"), xorstr_("Muzzle Boost") };

							// Ensure the attachment vector has the correct size
							if (weapon.attachments.size() < 3)
								weapon.attachments.resize(3, 0);

							if (ImGui::Combo(xorstr_("Sight"), &weapon.attachments[0], RustSights, IM_ARRAYSIZE(RustSights))) {
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Grip"), &weapon.attachments[1], RustGrips, IM_ARRAYSIZE(RustGrips))) {
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Barrel"), &weapon.attachments[2], RustBarrels, IM_ARRAYSIZE(RustBarrels))) {
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}
							break;
						}
					case 3: // Overwatch
						{
							break;
						}
					}
				}
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::SeparatorText(xorstr_("Recoil Pattern Management"));

			ImGui::Checkbox(xorstr_("Edit Recoil Pattern"), &editingPattern);

			if (editingPattern) {
				std::vector<weaponData>* currentWeaponData = nullptr;
				if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
					currentWeaponData = &settings.characters[settings.activeState.selectedCharacterIndex].weapondata;
				}

				if (currentWeaponData && !currentWeaponData->empty()) {
					int selectedWeaponIndex = settings.activeState.isPrimaryActive ?
						settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] :
						settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1];

					selectedWeaponIndex = std::min(selectedWeaponIndex, static_cast<int>(currentWeaponData->size()) - 1);

					auto& weapon = (*currentWeaponData)[selectedWeaponIndex];

					if (ImGui::Button(xorstr_("Add Point"))) {
						weapon.values.push_back({ 0.0f, 0.0f, 1.0f });
						globals.filesystem.unsavedChanges = true;
						settings.activeState.weaponDataChanged = true;
					}

					ImGui::SameLine();

					if (ImGui::Button(xorstr_("Reset Pattern"))) {
						ImGui::OpenPopup(xorstr_("Reset Pattern Confirmation"));
					}

					if (ImGui::BeginPopupModal(xorstr_("Reset Pattern Confirmation"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
						ImGui::Text(xorstr_("Are you sure you want to reset the recoil pattern?"));
						ImGui::Text(xorstr_("This action cannot be undone."));

						if (ImGui::Button(xorstr_("Yes"), ImVec2(120, 0))) {
							// Create a default pattern with 3 points
							weapon.values = {
								{0.0f, 5.0f, 1000.0f},
								{0.0f, 5.0f, 1000.0f},
								{0.0f, 5.0f, 1000.0f}
							};
							globals.filesystem.unsavedChanges = true;
							settings.activeState.weaponDataChanged = true;
							ImGui::CloseCurrentPopup();
						}

						ImGui::SameLine();

						if (ImGui::Button(xorstr_("No"), ImVec2(120, 0))) {
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					ImGui::BeginChild(xorstr_("PatternEditor"), ImVec2(0, 200), true);

					ImGui::Columns(4, xorstr_("PatternColumns"), true);
					ImGui::Text(xorstr_("Point")); ImGui::NextColumn();
					ImGui::Text(xorstr_("X Offset")); ImGui::NextColumn();
					ImGui::Text(xorstr_("Y Offset")); ImGui::NextColumn();
					ImGui::Text(xorstr_("Duration (ms)")); ImGui::NextColumn();
					ImGui::Separator();

					for (size_t i = 0; i < weapon.values.size(); i++) {
						ImGui::PushID(static_cast<int>(i));

						// Point number
						ImGui::Text("%zu", i + 1);
						ImGui::NextColumn();

						// X value
						ImGui::SetNextItemWidth(-1);
						if (ImGui::InputFloat(xorstr_("##X"), &weapon.values[i][0], 0.1f, 1.0f, "%.1f")) {
							globals.filesystem.unsavedChanges = true;
							settings.activeState.weaponDataChanged = true;
						}
						ImGui::NextColumn();

						// Y value
						ImGui::SetNextItemWidth(-1);
						if (ImGui::InputFloat(xorstr_("##Y"), &weapon.values[i][1], 0.1f, 1.0f, "%.1f")) {
							globals.filesystem.unsavedChanges = true;
							settings.activeState.weaponDataChanged = true;
						}
						ImGui::NextColumn();

						// Duration value
						ImGui::SetNextItemWidth(-1);
						if (ImGui::InputFloat(xorstr_("##Duration"), &weapon.values[i][2], 100.0f, 500.0f, "%.0f")) {
							if (weapon.values[i][2] < 1.0f) weapon.values[i][2] = 1.0f; // Minimum duration
							globals.filesystem.unsavedChanges = true;
							settings.activeState.weaponDataChanged = true;
						}
						ImGui::SameLine();

						if (ImGui::Button(xorstr_("X"))) {
							if (weapon.values.size() > 1) { // Don't allow empty patterns
								weapon.values.erase(weapon.values.begin() + i);
								i--;
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}
						}
						ImGui::NextColumn();

						ImGui::PopID();
					}

					ImGui::Columns(1);
					ImGui::EndChild();

					if (ImGui::Button(xorstr_("Import Pattern from CSV"))) {
						ImGui::OpenPopup(xorstr_("Import Pattern"));
					}

					ImGui::SameLine();

					if (ImGui::Button(xorstr_("Export Pattern to CSV"))) {
						ImGui::OpenPopup(xorstr_("Export Pattern"));
					}

					if (ImGui::BeginPopupModal(xorstr_("Import Pattern"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
						static char importBuffer[4096] = "";

						ImGui::Text(xorstr_("Paste CSV data in format: X,Y,Duration"));
						ImGui::Text(xorstr_("One point per line. Example: 0.5,2.3,1000"));

						ImGui::InputTextMultiline(xorstr_("##ImportText"), importBuffer, IM_ARRAYSIZE(importBuffer),
							ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10));

						if (ImGui::Button(xorstr_("Import"), ImVec2(120, 0))) {
							// Parse the CSV input
							std::vector<std::vector<float>> newPattern;
							std::istringstream ss(importBuffer);
							std::string line;

							while (std::getline(ss, line)) {
								std::vector<float> point = { 0.0f, 0.0f, 1000.0f };
								std::istringstream lineStream(line);
								std::string cell;
								int idx = 0;

								while (std::getline(lineStream, cell, ',') && idx < 3) {
									try {
										point[idx++] = std::stof(cell);
									}
									catch (...) {
										// Ignore parsing errors
									}
								}

								newPattern.push_back(point);
							}

							if (!newPattern.empty()) {
								weapon.values = newPattern;
								globals.filesystem.unsavedChanges = true;
								settings.activeState.weaponDataChanged = true;
							}

							memset(importBuffer, 0, sizeof(importBuffer));
							ImGui::CloseCurrentPopup();
						}

						ImGui::SameLine();

						if (ImGui::Button(xorstr_("Cancel"), ImVec2(120, 0))) {
							memset(importBuffer, 0, sizeof(importBuffer));
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					if (ImGui::BeginPopupModal(xorstr_("Export Pattern"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
						std::string exportText;

						for (const auto& point : weapon.values) {
							exportText += std::to_string(point[0]) + "," +
								std::to_string(point[1]) + "," +
								std::to_string(point[2]) + "\n";
						}

						static char exportBuffer[4096];
						strncpy(exportBuffer, exportText.c_str(), sizeof(exportBuffer));
						exportBuffer[sizeof(exportBuffer) - 1] = '\0';

						ImGui::Text(xorstr_("Copy this data to clipboard:"));
						ImGui::InputTextMultiline(xorstr_("##ExportText"), exportBuffer,
							IM_ARRAYSIZE(exportBuffer),
							ImVec2(-1.0f, ImGui::GetTextLineHeight() * 10),
							ImGuiInputTextFlags_ReadOnly);

						if (ImGui::Button(xorstr_("Close"), ImVec2(120, 0))) {
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}
				}
			}

			ImGui::EndChild();

			ImGui::NextColumn();

			// Right column: Recoil pattern display
			ImGui::BeginChild(xorstr_("RecoilPatternDisplay"));

			std::vector<weaponData>* currentWeaponData = nullptr;
			if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
				currentWeaponData = &settings.characters[settings.activeState.selectedCharacterIndex].weapondata;
			}

			if (currentWeaponData && !currentWeaponData->empty()) {
				int selectedWeaponIndex = settings.activeState.isPrimaryActive ?
					settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] :
					settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1];

				selectedWeaponIndex = std::min(selectedWeaponIndex, static_cast<int>(currentWeaponData->size()) - 1);

				auto& weapon = (*currentWeaponData)[selectedWeaponIndex];

				// Display pattern statistics
				ImGui::Text(xorstr_("Pattern Statistics:"));

				float totalXOffset = 0.0f;
				float totalYOffset = 0.0f;
				float totalDuration = 0.0f;

				for (const auto& point : weapon.values) {
					totalXOffset += point[0];
					totalYOffset += point[1];
					totalDuration += point[2];
				}

				ImGui::Text(xorstr_("Total X Offset: %.1f"), totalXOffset);
				ImGui::Text(xorstr_("Total Y Offset: %.1f"), totalYOffset);
				ImGui::Text(xorstr_("Total Duration: %.0f ms"), totalDuration);
				ImGui::Text(xorstr_("Number of Points: %zu"), weapon.values.size());

				ImGui::Spacing();

				// Draw the recoil pattern visualization
				DrawRecoilPattern(weapon.values);
			}
			else {
				ImGui::Text(xorstr_("No weapon selected or data available."));
			}

			ImGui::EndChild();

			ImGui::Columns(1);

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(xorstr_("Config"))) {
			ImGui::Columns(2, xorstr_("ConfigColumns"), false);

			// Left column: Config list and file operations
			ImGui::BeginChild(xorstr_("ConfigList"), ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

			if (ImGui::Button(xorstr_("Refresh List"))) {
				globals.filesystem.configFiles = ut.scanCurrentDirectoryForConfigFiles();
			}

			ImGui::SameLine();

			if (ImGui::Button(xorstr_("New Config"))) {
				NewConfigPopup = true;
			}

			ImGui::Separator();

			// Styled config list
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::BeginChild(xorstr_("ConfigList"), ImVec2(0, 0), true);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
			for (int i = 0; i < globals.filesystem.configFiles.size(); i++) {
				bool isLegacy = settings.isLegacyConfig(globals.filesystem.configFiles[i]);
				const bool is_selected = (globals.filesystem.activeFileIndex == i);

				// Push a different style color for legacy configs
				if (isLegacy) {
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f)); // Red-ish for legacy
				}

				// Add [Legacy] tag to the name
				std::string displayName = globals.filesystem.configFiles[i];
				if (isLegacy) {
					displayName += " [Legacy]";
				}

				if (ImGui::Selectable(displayName.c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
					globals.filesystem.activeFileIndex = i;
					if (ImGui::IsMouseDoubleClicked(0) && !isLegacy) {
						// Only allow double-click loading for non-legacy configs
						if (globals.filesystem.unsavedChanges) {
							loadConfigPopup = true;
						}
						else {
							globals.filesystem.activeFile = globals.filesystem.configFiles[i];
							settings.activeState.selectedCharacterIndex = 0;
							settings.activeState.weaponOffOverride = false;
							settings.readSettings(globals.filesystem.activeFile, true, true);
							globals.filesystem.unsavedChanges = false;
						}
					}
				}

				// Restore style for legacy configs
				if (isLegacy) {
					ImGui::PopStyleColor();
				}

				if (is_selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::PopStyleVar(); // ItemSpacing

			ImGui::EndChild();
			ImGui::PopStyleColor(); // Border color
			ImGui::PopStyleVar(); // ChildRounding, ChildBorderSize

			ImGui::EndChild();

			if (ImGui::Button(xorstr_("Load"))) {
				if (globals.filesystem.activeFileIndex < globals.filesystem.configFiles.size()) {
					std::string selectedFile = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
					// Check if selected file is a legacy config
					if (settings.isLegacyConfig(selectedFile)) {
						// Show popup that legacy configs cannot be loaded
						ImGui::OpenPopup(xorstr_("Cannot Load Legacy Config"));
					}
					else if (globals.filesystem.unsavedChanges) {
						loadConfigPopup = true;
					}
					else {
						globals.filesystem.activeFile = selectedFile;
						settings.activeState.selectedCharacterIndex = 0;
						settings.activeState.weaponOffOverride = false;
						settings.readSettings(globals.filesystem.activeFile, true, true);
						globals.filesystem.unsavedChanges = false;
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button(xorstr_("Save"))) {
				if (globals.filesystem.activeFileIndex >= 0 && globals.filesystem.activeFileIndex < globals.filesystem.configFiles.size()) {
					std::string selectedFile = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
					if (selectedFile == globals.filesystem.activeFile) {
						// Saving to the same file, proceed normally
						settings.saveSettings(globals.filesystem.activeFile);
						globals.filesystem.unsavedChanges = false;
					}
					else {
						// Trying to save over a different config, open confirmation popup
						saveConfigPopup = true;
					}
				}
				else if (!globals.filesystem.activeFile.empty()) {
					// No file selected, but we have an active file, save normally
					settings.saveSettings(globals.filesystem.activeFile);
					globals.filesystem.unsavedChanges = false;
				}
			}

			ImGui::SameLine();

			if (ImGui::Button(xorstr_("Delete"))) {
				if (globals.filesystem.activeFileIndex >= 0 && globals.filesystem.activeFileIndex < globals.filesystem.configFiles.size()) {
					deleteConfigPopup = true;
				}
			}

			ImGui::SameLine();

			if (ImGui::Button(xorstr_("Rename"))) {
				if (globals.filesystem.activeFileIndex >= 0 && globals.filesystem.activeFileIndex < globals.filesystem.configFiles.size()) {
					RenameConfigPopup = true;
				}
			}

			ImGui::NextColumn();

			// Right column: Config details and conversion
			ImGui::BeginChild(xorstr_("ConfigDetails"), ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

			ImGui::Text(xorstr_("Current Config: %s"), globals.filesystem.activeFile.c_str());
			ImGui::Separator();

			ImGui::Text(xorstr_("Config Details:"));
			ImGui::Text(xorstr_("Characters: %d"), settings.characters.size());
			ImGui::Text(xorstr_("Potato Mode: %s"), settings.globalSettings.potato ? xorstr_("Enabled") : xorstr_("Disabled"));
			//TODO: Add more config details

			ImGui::EndChild();

			ImGui::Columns(1);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	globals.initshutdown = !inverseShutdown;

	if (globals.initshutdown) {
		if (!globals.filesystem.unsavedChanges) {
			globals.done = true;
		}
		else {
			initshutdownpopup = true;
		}
	}

	popup(loadConfigPopup, 0);
	popup(saveConfigPopup, 1);
	popup(initshutdownpopup, 2);
	popup(deleteConfigPopup, 3);

	newConfigPopup(NewConfigPopup, newConfigName);
	renameConfigPopup(RenameConfigPopup, renameBuffer);

	overlay.UpdateState();

	ImGui::End();
}