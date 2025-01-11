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

void newConfigPopup(bool trigger, char* newConfigName, int& selectedMode, int& selectedGame) {
	if (trigger) {
		ImGui::OpenPopup(xorstr_("NewConfigPopup"));
	}

	const char* games[] = { "Siege", "Rust", "Overwatch" };

	if (ImGui::BeginPopup(xorstr_("NewConfigPopup"))) {
		ImGui::InputText(xorstr_("Config Name"), newConfigName, 256);

		const char* modes[] = { "Generic", "Character", "Game" };
		ImGui::Combo(xorstr_("Mode"), &selectedMode, modes, IM_ARRAYSIZE(modes));

		if (selectedMode == 2) {  // If "Game" mode is selected
			ImGui::Combo(xorstr_("Game"), &selectedGame, games, IM_ARRAYSIZE(games));
		}

		if (ImGui::Button(xorstr_("Create"))) {
			std::string newFileName = std::string(newConfigName) + xorstr_(".focus");
			Settings newSettings;
			newSettings.mode = modes[selectedMode];
			newSettings.potato = true;
			newSettings.aimbotData = { 0, 0, false, 0, 10, 10, 1, 200, true, { 0, 0.02f, 0.2f, 0.008f, 1.f }, { 0, 3, 0, 20, 10, 80, 10, 5 }, { 0, 0, 10, false } }; // Default aimbot settings

			// Pre-fill with example values based on mode and game
			if (newSettings.mode == xorstr_("Generic")) {
				characterData genericChar;
				weaponData exampleWeapon;
				exampleWeapon.weaponname = xorstr_("Example Weapon");
				exampleWeapon.rapidfire = true;
				exampleWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
				genericChar.weapondata.push_back(exampleWeapon);
				genericChar.selectedweapon = { 0, 0 };
				newSettings.characters.push_back(genericChar);
			}
			else if (newSettings.mode == xorstr_("Character")) {
				newSettings.wpn_keybinds = { xorstr_("0x31"), xorstr_("0x32") }; // Example keybinds (1 and 2 keys)
				newSettings.aux_keybinds = { xorstr_("0x58") }; // Example aux keybind (X key)

				characterData exampleChar;
				exampleChar.charactername = xorstr_("Example Character");
				exampleChar.options = { true, false, true, false, false };

				weaponData primaryWeapon;
				primaryWeapon.weaponname = xorstr_("Primary Weapon");
				primaryWeapon.rapidfire = true;
				primaryWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
				exampleChar.weapondata.push_back(primaryWeapon);

				weaponData secondaryWeapon;
				secondaryWeapon.weaponname = xorstr_("Secondary Weapon");
				secondaryWeapon.rapidfire = false;
				secondaryWeapon.values = { {0.05f, 0.05f, 1.0f}, {0.1f, 0.1f, 1.0f} };
				exampleChar.weapondata.push_back(secondaryWeapon);

				exampleChar.selectedweapon = { 0, 1 }; // Primary and secondary weapon indices
				newSettings.characters.push_back(exampleChar);
			}
			else if (newSettings.mode == xorstr_("Game")) {
				newSettings.game = games[selectedGame];

				if (newSettings.game == xorstr_("Siege")) {
					characterData exampleChar;
					exampleChar.charactername = xorstr_("Example Operator");
					exampleChar.options = { true, false };

					weaponData primaryWeapon;
					primaryWeapon.weaponname = xorstr_("Primary Weapon");
					primaryWeapon.rapidfire = true;
					primaryWeapon.attachments = { 0, 0, 0 };
					primaryWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
					exampleChar.weapondata.push_back(primaryWeapon);

					weaponData secondaryWeapon;
					secondaryWeapon.weaponname = xorstr_("Secondary Weapon");
					secondaryWeapon.rapidfire = false;
					secondaryWeapon.attachments = { 0, 0, 0 };
					secondaryWeapon.values = { {0.05f, 0.05f, 1.0f}, {0.1f, 0.1f, 1.0f} };
					exampleChar.weapondata.push_back(secondaryWeapon);

					exampleChar.selectedweapon = { 0, 1 };
					newSettings.characters.push_back(exampleChar);
					newSettings.sensitivity = { 50.0f, 50.0f, 100.0f, 100.0f, 100.0f, 0.02f };
				}
				else if (newSettings.game == xorstr_("Rust")) {
					characterData rustChar;
					rustChar.options = { true };

					weaponData exampleWeapon;
					exampleWeapon.weaponname = xorstr_("Example Weapon");
					exampleWeapon.rapidfire = true;
					exampleWeapon.attachments = { 0, 0, 0 };
					exampleWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
					rustChar.weapondata.push_back(exampleWeapon);

					rustChar.selectedweapon = { 0, 0 };
					newSettings.characters.push_back(rustChar);
					newSettings.sensitivity = { 0.5f, 1.0f };
				}
				else if (newSettings.game == xorstr_("Overwatch")) {
					characterData owChar;
					owChar.options = { true };

					weaponData exampleWeapon;
					exampleWeapon.weaponname = xorstr_(".Disabled");
					exampleWeapon.rapidfire = false;
					exampleWeapon.attachments = { 0, 0, 0 };
					exampleWeapon.values = { {0.f, 0.f, 0.f} };
					owChar.weapondata.push_back(exampleWeapon);

					owChar.selectedweapon = { 0, 0 };
					newSettings.characters.push_back(owChar);
					newSettings.sensitivity = { 1.0f, 1.0f };
				}
			}

			newSettings.saveSettings(newFileName);
			globals.filesystem.configFiles.push_back(newFileName);

			memset(newConfigName, 0, sizeof(newConfigName));
			selectedMode = 0;
			selectedGame = 0;

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(xorstr_("Cancel"))) {
			memset(newConfigName, 0, sizeof(newConfigName));
			selectedMode = 0;
			selectedGame = 0;

			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void renameConfigPopup(bool& trigger, static char renameBuffer[256]) {
	if (trigger) {
		ImGui::OpenPopup(xorstr_("RenamePopup"));
	}

	if (ImGui::BeginPopupModal(xorstr_("RenamePopup"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
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
				settings.selectedCharacterIndex = 0;
				settings.weaponOffOverride = false;
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

void weaponKeyHandler() {
	static bool weaponPressedOld = false;
	bool weaponPressed = false;

	for (const auto& keybind : settings.wpn_keybinds) {
		if (GetAsyncKeyState(std::stoi(keybind, nullptr, 0))) {
			if (!weaponPressedOld) {
				settings.isPrimaryActive = !settings.isPrimaryActive;
				weaponPressedOld = true;
			}

			weaponPressed = true;
		}
	}

	if (!weaponPressed) {
		weaponPressedOld = false; // Reset the flag if no key was pressed
	}
}

// Low-level mouse hook procedure
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION) {
		PMSLLHOOKSTRUCT pHookStruct = (PMSLLHOOKSTRUCT)lParam;
		if (wParam == WM_MOUSEWHEEL && settings.mode == xorstr_("Character")) {
			if (settings.characters[settings.selectedCharacterIndex].options[2]) {
				settings.isPrimaryActive = !settings.isPrimaryActive;
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void Menu::mouseScrollHandler()
{
	HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
	if (mouseHook == NULL) {
		std::cerr << xorstr_("Failed to set up mouse hook.") << std::endl;
		return;
	}

	// Message loop
	MSG msg;
	while (!globals.shutdown) {
		// Check for messages with a timeout of 0
		DWORD result = MsgWaitForMultipleObjects(0, NULL, FALSE, 0, QS_ALLINPUT);
		if (result == WAIT_OBJECT_0) {
			// There are messages in the queue, process them
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else if (result == WAIT_FAILED) {
			std::cerr << xorstr_("MsgWaitForMultipleObjects error") << std::endl;
			break;
		}
		// No messages in the queue

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	UnhookWindowsHookEx(mouseHook);

	return;
}

void auxKeyHandler() {
	static bool weaponPressedOld = false;
	bool weaponPressed = false;

	for (const auto& keybind : settings.aux_keybinds) {
		if (GetAsyncKeyState(std::stoi(keybind, nullptr, 0))) {
			if (!weaponPressedOld) {
				settings.weaponOffOverride = !settings.weaponOffOverride;
				weaponPressedOld = true;
			}

			weaponPressed = true;
		}
	}

	if (!weaponPressed) {
		weaponPressedOld = false; // Reset the flag if no key was pressed
	}
}

std::vector<float> calculateSensitivityModifierR6() {
	float oldBaseSens = 10;
	float oldRelativeSens = 50;
	float oldMultiplier = 0.02f;
	float oldFov = 82.0f;

	float newBaseXSens = settings.sensitivity[0];
	float newBaseYSens = settings.sensitivity[1];
	float new1xSens = settings.sensitivity[2];
	float new25xSens = settings.sensitivity[3];
	float new35xSens = settings.sensitivity[4];
	float newMultiplier = settings.sensitivity[5];

	float newFov = settings.fov;

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

	settings.fovSensitivityModifier = fovModifier;

	if (settings.selectedCharacterIndex < settings.characters.size() &&
		!settings.characters[settings.selectedCharacterIndex].weapondata.empty()) {

		int weaponIndex = settings.isPrimaryActive ?
			settings.characters[settings.selectedCharacterIndex].selectedweapon[0] :
			settings.characters[settings.selectedCharacterIndex].selectedweapon[1];

		weaponIndex = std::min(weaponIndex, static_cast<int>(settings.characters[settings.selectedCharacterIndex].weapondata.size()) - 1);

		const auto& weapon = settings.characters[settings.selectedCharacterIndex].weapondata[weaponIndex];

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

	float newSensXModifier = ((oldBaseSens * oldRelativeSens * oldMultiplier) / (scopeModifier * newBaseXSens * newMultiplier) * totalAttachXEffect) * fovModifier;
	float newSensYModifier = ((oldBaseSens * oldRelativeSens * oldMultiplier) / (scopeModifier * newBaseYSens * newMultiplier) * totalAttachYEffect) * fovModifier;

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

	float newBaseSens = settings.sensitivity[0];
	float newADSSens = settings.sensitivity[1];
	float newFov = settings.fov;
	float sightEffect = 1.0f;
	float gripEffect = 1.0f;
	float barrelEffect = 1.0f;

	float fovDifference = newFov - oldFov;
	float fovModifier = 1.0f + (quadratic_coef * fovDifference * fovDifference) +
		(linear_coef * fovDifference);

	settings.fovSensitivityModifier = fovModifier;

	if (settings.selectedCharacterIndex < settings.characters.size() &&
		!settings.characters[settings.selectedCharacterIndex].weapondata.empty()) {

		const auto& character = settings.characters[settings.selectedCharacterIndex];
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

	float sensModifier = ((oldBaseSens * oldADSSens) / (newBaseSens * newADSSens) * totalAttachEffect * (standingModifier * movingModifier)) * fovModifier;

	return std::vector<float>{ sensModifier, sensModifier };
}

std::vector<float> calculateSensitivityModifierOverwatch() {
	settings.fovSensitivityModifier = 1;

	return std::vector<float>{ 1.f, 1.f };
}

// Function to parse keybinds and update global struct
void keybindManager() {

	settings.misc.hotkeys.Update();

	if (settings.mode == xorstr_("Generic")) {
		settings.isPrimaryActive = true;
		settings.sensMultiplier = { 1.f, 1.f };
	}
	else if (settings.mode == xorstr_("Character")) {

		if (settings.characters[settings.selectedCharacterIndex].options[0]) {

			if (!globals.capture.desktopMat.empty()) {
				// Define ratios for crop region
				float cropRatioX = 0.8f; // 20% from left
				float cropRatioY = 0.82f; // 20% from top
				float cropRatioWidth = 0.18f; // 18% of total width
				float cropRatioHeight = 0.14f; // 14% of total height

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

				dx.detectWeaponR6(smallRegion, 25, 75);
			}
		}
		else {
			settings.weaponOffOverride = false;
			settings.isPrimaryActive = true;
		}

		if (settings.characters[settings.selectedCharacterIndex].options[1]) {
			weaponKeyHandler();
		}

		// Scrolling options is handled in its thread

		if (settings.characters[settings.selectedCharacterIndex].options[3]) {
			auxKeyHandler();
		}

		settings.sensMultiplier = { 1.f, 1.f };
	}
	else if (settings.mode == xorstr_("Game")) {
		if (settings.game == xorstr_("Siege")) {
			if (settings.characters[settings.selectedCharacterIndex].options[0]) {

				if (!globals.capture.desktopMat.empty()) {
					// Define ratios for crop region
					float cropRatioX = 0.0f;
					float cropRatioY = 0.82f;
					float cropRatioWidth = 0.008f;
					float cropRatioHeight = 0.14f;

					switch (settings.aspect_ratio) {
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
			}
			else {
				settings.weaponOffOverride = false;
				settings.isPrimaryActive = true;
			}

			if (settings.characters[settings.selectedCharacterIndex].options[2]) {
				static bool detectedOperator = false;
				static bool useShootingRangeOffset = false;

				float cropRatioX = 0.f;
				float cropRatioY = 0.0f;
				float cropRatioWidth = 0.f;
				float cropRatioHeight = 0.052f;

				// Define ratios for crop region
				switch (settings.aspect_ratio) {
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

			settings.sensMultiplier = calculateSensitivityModifierR6();
		}
		else if (settings.game == xorstr_("Rust")) {
			static bool initializeRustDetector = false;

			if (settings.characters[settings.selectedCharacterIndex].options[0]) {
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

			settings.sensMultiplier = calculateSensitivityModifierRust();

			settings.isPrimaryActive = true;
		}
	}
}

void DrawRecoilPattern(const std::vector<std::vector<float>>& recoilData) {
	if (recoilData.empty()) return;

	std::vector<float> sens = settings.sensMultiplier;

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

	//g.filesystem.unsavedChanges = ut.isEdited(g.characterinfo.jsonData, editor.GetText());

	keybindManager();

	if (ImGui::BeginTabBar(xorstr_("##TabBar")))
	{
		if (settings.mode == xorstr_("Generic")) {
			if (ImGui::BeginTabItem(xorstr_("Generic"))) {

				ImGui::Columns(2);
				ImGui::BeginChild(xorstr_("Left Column"));

				if (settings.characters.size() > 0) {

					if (comboBoxWep(xorstr_("Weapon"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[0], settings.characters)) {
						settings.weaponDataChanged = true;
					}

					if (ImGui::Checkbox(xorstr_("Rapid Fire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].rapidfire)) {
						settings.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}

					if (ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato)) {
						globals.filesystem.unsavedChanges = true;
					}
				}
				else {
					ImGui::Text(xorstr_("Please load a weapons file"));
				}

				ImGui::EndChild();
				ImGui::NextColumn();
				ImGui::BeginChild(xorstr_("Right Column"));

				if (settings.characters.size() > 0 && settings.selectedCharacterIndex < settings.characters.size()) {
					const auto& activeWeapon = settings.isPrimaryActive ?
						settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]] :
						settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]];

					DrawRecoilPattern(activeWeapon.values);
				}
				else {
					ImGui::Text(xorstr_("No weapon selected or data available."));
				}

				ImGui::EndChild();
				ImGui::Columns(1);

				ImGui::EndTabItem();
			}
		}
		else if (settings.mode == xorstr_("Character")) {
			if (ImGui::BeginTabItem(xorstr_("Character"))) {

				ImGui::Columns(2);
				ImGui::BeginChild(xorstr_("Left Column"));

				if (settings.characters.size() > 0) {

					if (comboBoxChar(xorstr_("Character"), settings.selectedCharacterIndex, settings.characters)) {
						settings.weaponDataChanged = true;
					}

					ImGui::Spacing();
					ImGui::Spacing();

					if (settings.weaponOffOverride) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
						ImGui::SeparatorText(xorstr_("Primary"));
						ImGui::PopStyleColor();
					}
					else if (settings.isPrimaryActive) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						ImGui::SeparatorText(xorstr_("Primary"));
						ImGui::PopStyleColor();
					}
					else {
						ImGui::SeparatorText(xorstr_("Primary"));
					}

					if (comboBoxWep(xorstr_("Primary"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[0], settings.characters)) {
						settings.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}
					
					if (ImGui::Checkbox(xorstr_("Primary Rapid Fire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].rapidfire)) {
						settings.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}

					ImGui::Spacing();
					ImGui::Spacing();

					if (settings.weaponOffOverride) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
						ImGui::SeparatorText(xorstr_("Secondary"));
						ImGui::PopStyleColor();
					}
					else if (!settings.isPrimaryActive) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						ImGui::SeparatorText(xorstr_("Secondary"));
						ImGui::PopStyleColor();
					}
					else {
						ImGui::SeparatorText(xorstr_("Secondary"));
					}

					if (comboBoxWep(xorstr_("Secondary"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[1], settings.characters)) {
						settings.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}

					if (ImGui::Checkbox(xorstr_("Secondary Rapid Fire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].rapidfire)) {
						settings.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SeparatorText(xorstr_("Options"));

					std::vector<const char*> MultiOptions = { xorstr_("R6 Auto Weapon Detection"), xorstr_("Manual Weapon Detection"), xorstr_("Scroll Detection"), xorstr_("Aux Disable"), xorstr_("Gadget Detection Override") };

					if (multiCombo(xorstr_("Options"), MultiOptions, settings.characters[settings.selectedCharacterIndex].options)) {
						settings.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}

					if (ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato)) {
						globals.filesystem.unsavedChanges = true;
					}
				}
				else {
					ImGui::Text(xorstr_("Please load a weapons file"));
				}

				ImGui::EndChild();
				ImGui::NextColumn();
				ImGui::BeginChild(xorstr_("Right Column"));

				if (settings.characters.size() > 0 && settings.selectedCharacterIndex < settings.characters.size()) {
					const auto& activeWeapon = settings.isPrimaryActive ?
						settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]] :
						settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]];

					DrawRecoilPattern(activeWeapon.values);
				}
				else {
					ImGui::Text(xorstr_("No weapon selected or data available."));
				}

				ImGui::EndChild();
				ImGui::Columns(1);

				ImGui::EndTabItem();
			}
		}
		else if (settings.mode == xorstr_("Game")) {
			if (settings.game == xorstr_("Siege")) {
				if (ImGui::BeginTabItem(xorstr_("Game"))) {

					ImGui::Columns(2);
					ImGui::BeginChild(xorstr_("Left Column"));

					if (settings.characters.size() > 0) {
						ImGui::Text(xorstr_("Game: %s"), settings.game.c_str());

						if (comboBoxChar(xorstr_("Character"), settings.selectedCharacterIndex, settings.characters)) {
							settings.weaponDataChanged = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();

						if (settings.weaponOffOverride) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
							ImGui::SeparatorText(xorstr_("Primary"));
							ImGui::PopStyleColor();
						}
						else if (settings.isPrimaryActive) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
							ImGui::SeparatorText(xorstr_("Primary"));
							ImGui::PopStyleColor();
						}
						else {
							ImGui::SeparatorText(xorstr_("Primary"));
						}

						const char* Sights[] = { xorstr_("1x"), xorstr_("2.5x"), xorstr_("3.5x") };
						const char* Grips[] = { xorstr_("Horizontal/Angled/None"), xorstr_("Vertical") };
						const char* Barrels[] = { xorstr_("Supressor/Extended/None"), xorstr_("Muzzle Break (Semi-Auto Only)"), xorstr_("Compensator"), xorstr_("Flash Hider") };

						if (comboBoxWep(xorstr_("Primary"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[0], settings.characters)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (ImGui::Checkbox(xorstr_("Primary Rapid Fire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].rapidfire)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Hold Left+Right mouse to automatically shoot"));
						if (comboBoxGen(xorstr_("Primary Sight"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[0], Sights, 3)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Grip"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[1], Grips, 2)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Primary Barrel"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[2], Barrels, 4)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();

						if (settings.weaponOffOverride) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
							ImGui::SeparatorText(xorstr_("Secondary"));
							ImGui::PopStyleColor();
						}
						else if (!settings.isPrimaryActive) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
							ImGui::SeparatorText(xorstr_("Secondary"));
							ImGui::PopStyleColor();
						}
						else {
							ImGui::SeparatorText(xorstr_("Secondary"));
						}

						if (comboBoxWep(xorstr_("Secondary"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[1], settings.characters)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (ImGui::Checkbox(xorstr_("Secondary Rapid Fire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].rapidfire)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Hold Left+Right mouse to automatically shoot"));
						if (comboBoxGen(xorstr_("Secondary Sight"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[0], Sights, 3)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Secondary Grip"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[1], Grips, 2)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Secondary Barrel"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[2], Barrels, 4)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Options"));

						std::vector<const char*> MultiOptions = { xorstr_("R6 Auto Weapon Detection"), xorstr_("Gadget Detection Override"), xorstr_("R6 Operator Detection") };

						if (multiCombo(xorstr_("Options"), MultiOptions, settings.characters[settings.selectedCharacterIndex].options)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Weapon Detection - Automatically swaps between the selected primary and secondary\nGadget Detection Override - Prevents gadgets from disabling weapon detection\nOperator Detection - Automatically selects the operator after spawning in"));

						if (ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato)) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Significantly reduces CPU usage but causes more first-shot recoil"));

						const char* Aspect_Ratios[] = { xorstr_("16:9"), xorstr_("4:3"), xorstr_("5:4"), xorstr_("3:2"), xorstr_("16:10"), xorstr_("5:3"), xorstr_("19:10"), xorstr_("21:9") };

						if (comboBoxGen(xorstr_("Aspect Ratio"), &settings.aspect_ratio, Aspect_Ratios, 8)) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Settings -> Display -> Aspect Ratio"));

						ImGui::Spacing();

						if (ImGui::SliderFloat(xorstr_("FOV"), &settings.fov, 60.0f, 90.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Settings -> Display -> Field of View"));

						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Extra Data"));

						if (ImGui::SliderFloat(xorstr_("X Base Sensitivity"), &settings.sensitivity[0], 0.0f, 100.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Settings -> Controls -> Mouse Sensitivity Horizontal"));
						if (ImGui::SliderFloat(xorstr_("Y Base Sensitivity"), &settings.sensitivity[1], 0.0f, 100.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Settings -> Controls -> Mouse Sensitivity Vertical"));
						if (ImGui::SliderFloat(xorstr_("1x Sensitivity"), &settings.sensitivity[2], 0.0f, 200.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Settings -> Controls -> Mouse ADS Sensitivity -> 1.0x Magnification"));
						if (ImGui::SliderFloat(xorstr_("2.5x Sensitivity"), &settings.sensitivity[3], 0.0f, 200.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Settings -> Controls -> Mouse ADS Sensitivity -> 2.5x Magnification"));
						if (ImGui::SliderFloat(xorstr_("3.5x Sensitivity"), &settings.sensitivity[4], 0.0f, 200.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						tooltip(xorstr_("Settings -> Controls -> Mouse ADS Sensitivity -> 3.5x Magnification"));
						if (ImGui::SliderFloat(xorstr_("Sensitivity Multiplier"), &settings.sensitivity[5], 0.0f, 1.0f, xorstr_("%.3f"))) {
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Text(xorstr_("X Sensitivity Modifier: %f"), settings.sensMultiplier[0]);
						ImGui::Text(xorstr_("Y Sensitivity Modifier: %f"), settings.sensMultiplier[1]);
					}
					else {
						ImGui::Text(xorstr_("Please load a weapons file"));
					}

					ImGui::EndChild();
					ImGui::NextColumn();
					ImGui::BeginChild(xorstr_("Right Column"));

					if (settings.characters.size() > 0 && settings.selectedCharacterIndex < settings.characters.size()) {
						const auto& activeWeapon = settings.isPrimaryActive ?
							settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]] :
							settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]];

						DrawRecoilPattern(activeWeapon.values);
					}
					else {
						ImGui::Text(xorstr_("No weapon selected or data available."));
					}

					ImGui::EndChild();
					ImGui::Columns(1);

					ImGui::EndTabItem();
				}
			}
			else if (settings.game == xorstr_("Rust")) {
				if (ImGui::BeginTabItem(xorstr_("Game"))) {

					ImGui::Columns(2);
					ImGui::BeginChild(xorstr_("Left Column"));

					if (settings.characters.size() > 0) {
						ImGui::Text(xorstr_("Game: %s"), settings.game.c_str());

						const char* Sights[] = { xorstr_("None"), xorstr_("Handmade"), xorstr_("Holosight"), xorstr_("8x Scope"), xorstr_("16x Scope") };
						const char* Grips[] = { xorstr_("None"), xorstr_("Gas Compression Overdrive") };
						const char* Barrels[] = { xorstr_("None/Suppressor"), xorstr_("Muzzle Break"), xorstr_("Muzzle Boost") };

						if (comboBoxWep(xorstr_("Weapon"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[0], settings.characters)) {
							settings.weaponDataChanged = true;
						}
						if (ImGui::Checkbox(xorstr_("Rapid Fire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].rapidfire)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Sight"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[0], Sights, 5)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Grip"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[1], Grips, 2)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Barrel"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[2], Barrels, 3)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Options"));

						std::vector<const char*> MultiOptions = { xorstr_("Rust Auto Weapon Detection") };

						if (multiCombo(xorstr_("Options"), MultiOptions, settings.characters[settings.selectedCharacterIndex].options)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						if (ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato)) {
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();

						if (ImGui::SliderFloat(xorstr_("FOV"), &settings.fov, 70.0f, 90.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Extra Data"));

						if (ImGui::SliderFloat(xorstr_("Sensitivity"), &settings.sensitivity[0], 0.0f, 10.0f, xorstr_("%.3f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						if (ImGui::SliderFloat(xorstr_("Aiming Sensitivity"), &settings.sensitivity[1], 0.0f, 10.0f, xorstr_("%.3f"))) {
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Text(xorstr_("X Sensitivity Modifier: %f"), settings.sensMultiplier[0]);
						ImGui::Text(xorstr_("Y Sensitivity Modifier: %f"), settings.sensMultiplier[1]);
					}
					else {
						ImGui::Text(xorstr_("Please load a weapons file"));
					}

					ImGui::EndChild();
					ImGui::NextColumn();
					ImGui::BeginChild(xorstr_("Right Column"));

					if (settings.characters.size() > 0 && settings.selectedCharacterIndex < settings.characters.size()) {
						const auto& activeWeapon = settings.isPrimaryActive ?
							settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]] :
							settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]];

						DrawRecoilPattern(activeWeapon.values);
					}
					else {
						ImGui::Text(xorstr_("No weapon selected or data available."));
					}

					ImGui::EndChild();
					ImGui::Columns(1);

					ImGui::EndTabItem();
				}
			}
			else if (settings.game == xorstr_("Overwatch")) {
				if (ImGui::BeginTabItem(xorstr_("Game"))) {

					ImGui::Columns(2);
					ImGui::BeginChild(xorstr_("Left Column"));

					if (settings.characters.size() > 0) {
						ImGui::Text(xorstr_("Game: %s"), settings.game.c_str());

						if (comboBoxWep(xorstr_("Weapon"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[0], settings.characters)) {
							settings.weaponDataChanged = true;
						}
						if (ImGui::Checkbox(xorstr_("Rapid Fire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].rapidfire)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Options"));

						if (ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato)) {
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();

						if (ImGui::SliderFloat(xorstr_("FOV"), &settings.fov, 80.0f, 103.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Extra Data"));

						ImGui::Text(xorstr_("X Sensitivity Modifier: %f"), settings.sensMultiplier[0]);
						ImGui::Text(xorstr_("Y Sensitivity Modifier: %f"), settings.sensMultiplier[1]);
					}
					else {
						ImGui::Text(xorstr_("Please load a weapons file"));
					}

					ImGui::EndChild();
					ImGui::NextColumn();
					ImGui::BeginChild(xorstr_("Right Column"));

					if (settings.characters.size() > 0 && settings.selectedCharacterIndex < settings.characters.size()) {
						const auto& activeWeapon = settings.isPrimaryActive ?
							settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]] :
							settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]];

						DrawRecoilPattern(activeWeapon.values);
					}
					else {
						ImGui::Text(xorstr_("No weapon selected or data available."));
					}

					ImGui::EndChild();
					ImGui::Columns(1);

					ImGui::EndTabItem();
				}
			}
			else {
				ImGui::Text(xorstr_("Please load a valid game config"));
			}
		}

		if (settings.mode == xorstr_("Character") || settings.game == xorstr_("Siege") || settings.game == xorstr_("Rust") || settings.mode == xorstr_("Generic") || settings.game == xorstr_("Overwatch")) {
			if (ImGui::BeginTabItem(xorstr_("Aim"))) {

				if (settings.game == xorstr_("Overwatch")) {
					settings.aimbotData.type = 0;

					if (ImGui::Checkbox(xorstr_("Colour Aimbot"), &settings.aimbotData.enabled)) {
						globals.filesystem.unsavedChanges = true;
					}
				}
				else {
					settings.aimbotData.type = 1;

					if (!settings.aimbotData.enabled) {
						std::vector<const char*> providers = { xorstr_("CPU"), xorstr_("CUDA") };
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
							settings.pidDataChanged = true;
						}
						else if (settings.aimbotData.pidSettings.pidPreset == 1) {
							settings.aimbotData.pidSettings.proportional = 0.02f;
							settings.aimbotData.pidSettings.integral = 0.2f;
							settings.aimbotData.pidSettings.derivative = 0.008f;
							settings.aimbotData.pidSettings.rampUpTime = 1.f;
							settings.pidDataChanged = true;
						}
						else if (settings.aimbotData.pidSettings.pidPreset == 2) {
							settings.aimbotData.pidSettings.proportional = 0.035f;
							settings.aimbotData.pidSettings.integral = 0.8f;
							settings.aimbotData.pidSettings.derivative = 0.008f;
							settings.aimbotData.pidSettings.rampUpTime = 1.f;
							settings.pidDataChanged = true;
						}
					}

					if (settings.aimbotData.pidSettings.pidPreset == 3) {
						if (ImGui::InputFloat(xorstr_("Proportional"), &settings.aimbotData.pidSettings.proportional, 0.0f, 0.0f)) {
							globals.filesystem.unsavedChanges = true;
							settings.pidDataChanged = true;
						}
						tooltip(xorstr_("How quickly the aimbot moves"));
						if (ImGui::InputFloat(xorstr_("Integral"), &settings.aimbotData.pidSettings.integral, 0.0f, 0.0f)) {
							globals.filesystem.unsavedChanges = true;
							settings.pidDataChanged = true;
						}
						tooltip(xorstr_("How quickly the aimbot changes direction"));
						if (ImGui::InputFloat(xorstr_("Derivative"), &settings.aimbotData.pidSettings.derivative, 0.0f, 0.0f)) {
							globals.filesystem.unsavedChanges = true;
							settings.pidDataChanged = true;
						}
						tooltip(xorstr_("How far ahead in time the aimbot looks"));
						if (ImGui::InputFloat(xorstr_("Integral Ramp Up Time"), &settings.aimbotData.pidSettings.rampUpTime, 0.0f, 0.0f)) {
							globals.filesystem.unsavedChanges = true;
							settings.pidDataChanged = true;
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

					if (ImGui::SliderFloat(xorstr_("Triggerbot FOV"), &settings.aimbotData.triggerFov, 0, 10, xorstr_("%.1f%%"), 0.1f)) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("The percentage of the screen that the Triggerbot will fire within (Within the centre 10% of your screen)"));

					if (ImGui::SliderInt(xorstr_("Triggerbot Sleep"), &settings.aimbotData.triggerSleep, 0, 2000, xorstr_("%dms%"))) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("How long the Triggerbot will wait between shots"));

					if (ImGui::Checkbox(xorstr_("Limit Detector FPS"), &settings.aimbotData.limitDetectorFps)) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("Locks the aimbot detector to 66 FPS, this reduces CPU usage but will reduce accuracy"));
					
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
					}
				}

				ImGui::EndTabItem();
			}
		}

		if (settings.mode == xorstr_("Character") || settings.game == xorstr_("Siege") || settings.game == xorstr_("Rust") || settings.mode == xorstr_("Generic") || settings.game == xorstr_("Overwatch")) {
			if (ImGui::BeginTabItem(xorstr_("Misc"))) {
				std::vector<const char*> keysSettings = { xorstr_("Require Both"), xorstr_("Just Left Mouse"), xorstr_("Just Right Mouse"), xorstr_("Custom Key") };

				if (settings.game == xorstr_("Siege")) {
					if (settings.misc.hotkeys.RenderHotkey(xorstr_("Auto Quick-Peek"), HotkeyIndex::AutoQuickPeek)) {
						globals.filesystem.unsavedChanges = true;
					}

					if (settings.misc.hotkeys.RenderHotkey(xorstr_("Auto Hashom Peek"), HotkeyIndex::AutoHashomPeek)) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("Prone peeking\nMake sure the Prone Key is bound"));

					if (settings.misc.hotkeys.RenderHotkey(xorstr_("Prone Key"), HotkeyIndex::ProneKey)) {
						globals.filesystem.unsavedChanges = true;
					}

					if (ImGui::SliderInt(xorstr_("Auto Quick-Peek Delay"), &settings.misc.quickPeekDelay, 0, 200, xorstr_("%dms%"))) {
						globals.filesystem.unsavedChanges = true;
					}

					if (settings.misc.hotkeys.RenderHotkey(xorstr_("Fake Spinbot"), HotkeyIndex::FakeSpinBot)) {
						globals.filesystem.unsavedChanges = true;
					}

					if (settings.misc.hotkeys.RenderHotkey(xorstr_("Dim X Key"), HotkeyIndex::DimXKey)) {
						globals.filesystem.unsavedChanges = true;
					}
					tooltip(xorstr_("Stops internet traffic to/from Siege\nAllows you to clip through some objects and peek without being seen server-side"));
				}
				else if (settings.game == xorstr_("Rust")) {
					if (settings.misc.hotkeys.RenderHotkey(xorstr_("Crouch Key"), HotkeyIndex::CrouchKey)) {
						globals.filesystem.unsavedChanges = true;
					}
				}

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

				if (settings.misc.hotkeys.RenderHotkey(xorstr_("Hide UI Key"), HotkeyIndex::HideUiKey)) {
					globals.filesystem.unsavedChanges = true;
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(xorstr_("Config Editor")))
			{
				static bool editingPattern = false;

				ImGui::Columns(2, xorstr_("ConfigEditorColumns"), false);

				// Left column: All controls and editors
				ImGui::BeginChild(xorstr_("ControlsAndEditors"));

				if (!editingPattern)
				{
					if (settings.mode == xorstr_("Character") || settings.game == xorstr_("Siege"))
					{
						ImGui::SeparatorText(xorstr_("Characters"));

						// Character list
						ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
						ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
						ImGui::BeginChild(xorstr_("CharacterList"), ImVec2(0, 100), true);

						ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
						for (int i = 0; i < settings.characters.size(); i++)
						{
							const bool is_selected = (settings.selectedCharacterIndex == i);
							if (ImGui::Selectable(settings.characters[i].charactername.c_str(), is_selected))
							{
								settings.selectedCharacterIndex = i;
								settings.isPrimaryActive = true;
								settings.weaponDataChanged = true;
							}

							if (is_selected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::PopStyleVar(); // ItemSpacing

						ImGui::EndChild();
						ImGui::PopStyleColor(); // Border color
						ImGui::PopStyleVar(); // ChildBorderSize

						if (ImGui::Button(xorstr_("Add Character")))
						{
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
							settings.selectedCharacterIndex = settings.characters.size() - 1;
							settings.isPrimaryActive = true;
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::SameLine();

						if (ImGui::Button(xorstr_("Remove Character")) && settings.selectedCharacterIndex < settings.characters.size())
						{
							if (settings.characters.size() > 1) // Ensure there's always at least one character
							{
								settings.characters.erase(settings.characters.begin() + settings.selectedCharacterIndex);
								settings.selectedCharacterIndex = std::max(0, static_cast<int>(settings.characters.size()) - 1);
								settings.weaponDataChanged = true;
								globals.filesystem.unsavedChanges = true;
							}
							else
							{
								// Optionally, show a message to the user
								ImGui::OpenPopup(xorstr_("Cannot Remove Character"));
							}
						}

						if (ImGui::BeginPopupModal(xorstr_("Cannot Remove Character"), NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							ImGui::Text(xorstr_("Cannot remove the last character."));
							if (ImGui::Button(xorstr_("OK"), ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}

						if (settings.selectedCharacterIndex < settings.characters.size())
						{
							char characterNameBuffer[256];
							strncpy(characterNameBuffer, settings.characters[settings.selectedCharacterIndex].charactername.c_str(), sizeof(characterNameBuffer));
							characterNameBuffer[sizeof(characterNameBuffer) - 1] = '\0';

							if (ImGui::InputText(xorstr_("Character Name"), characterNameBuffer, sizeof(characterNameBuffer)))
							{
								if (strlen(characterNameBuffer) > 0)
								{
									settings.characters[settings.selectedCharacterIndex].charactername = characterNameBuffer;
									globals.filesystem.unsavedChanges = true;
								}
							}
						}

						ImGui::Spacing();
						ImGui::Spacing();
					}

					ImGui::SeparatorText(xorstr_("Weapons"));

					// Weapon list
					ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
					ImGui::BeginChild(xorstr_("WeaponList"), ImVec2(0, 100), true);

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
					std::vector<weaponData>* currentWeaponData = nullptr;
					if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust") || settings.game == xorstr_("Overwatch"))
					{
						if (!settings.characters.empty())
						{
							currentWeaponData = &settings.characters[0].weapondata;
						}
					}
					else if (settings.selectedCharacterIndex < settings.characters.size())
					{
						currentWeaponData = &settings.characters[settings.selectedCharacterIndex].weapondata;
					}

					if (currentWeaponData)
					{
						for (int i = 0; i < currentWeaponData->size(); i++)
						{
							const bool is_selected = (settings.isPrimaryActive ?
								settings.characters[settings.selectedCharacterIndex].selectedweapon[0] == i :
								settings.characters[settings.selectedCharacterIndex].selectedweapon[1] == i);

							const char* displayName = (*currentWeaponData)[i].weaponname.empty() ?
								xorstr_("Unnamed Weapon") :
								(*currentWeaponData)[i].weaponname.c_str();

							if (ImGui::Selectable(displayName, is_selected))
							{
								if (settings.isPrimaryActive)
									settings.characters[settings.selectedCharacterIndex].selectedweapon[0] = i;
								else
									settings.characters[settings.selectedCharacterIndex].selectedweapon[1] = i;

								settings.weaponDataChanged = true;
								globals.filesystem.unsavedChanges = true;
							}

							if (is_selected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
					}
					ImGui::PopStyleVar(); // ItemSpacing

					ImGui::EndChild();
					ImGui::PopStyleColor(); // Border color
					ImGui::PopStyleVar(); // ChildBorderSize

					if (ImGui::Button(xorstr_("Add Weapon")))
					{
						if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust") || settings.game == xorstr_("Overwatch"))
						{
							if (settings.characters.empty())
							{
								settings.characters.push_back(characterData());
							}
							settings.characters[0].weapondata.push_back(weaponData());
							settings.characters[0].weapondata.back().weaponname = xorstr_("New Weapon");
							// Add default recoil pattern
							std::vector<std::vector<float>> defaultPattern = {
								{0.0f, 5.0f, 1000.0f},
								{0.0f, 5.0f, 1000.0f},
								{0.0f, 5.0f, 1000.0f}
							};
							settings.characters[0].weapondata.back().values = defaultPattern;
							// Auto-select the new weapon
							settings.characters[0].selectedweapon[settings.isPrimaryActive ? 0 : 1] = settings.characters[0].weapondata.size() - 1;
						}
						else if (settings.selectedCharacterIndex < settings.characters.size())
						{
							settings.characters[settings.selectedCharacterIndex].weapondata.push_back(weaponData());
							settings.characters[settings.selectedCharacterIndex].weapondata.back().weaponname = xorstr_("New Weapon");
							// Add default recoil pattern
							std::vector<std::vector<float>> defaultPattern = {
								{0.0f, 5.0f, 1000.0f},
								{0.0f, 5.0f, 1000.0f},
								{0.0f, 5.0f, 1000.0f}
							};
							settings.characters[settings.selectedCharacterIndex].weapondata.back().values = defaultPattern;
							// Auto-select the new weapon
							settings.characters[settings.selectedCharacterIndex].selectedweapon[settings.isPrimaryActive ? 0 : 1] = settings.characters[settings.selectedCharacterIndex].weapondata.size() - 1;
						}
						settings.weaponDataChanged = true;
						globals.filesystem.unsavedChanges = true;
					}

					ImGui::SameLine();

					if (ImGui::Button(xorstr_("Remove Weapon")))
					{
						std::vector<weaponData>* currentWeaponData = nullptr;
						if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust") || settings.game == xorstr_("Overwatch"))
						{
							if (!settings.characters.empty())
							{
								currentWeaponData = &settings.characters[0].weapondata;
							}
						}
						else if (settings.selectedCharacterIndex < settings.characters.size())
						{
							currentWeaponData = &settings.characters[settings.selectedCharacterIndex].weapondata;
						}

						if (currentWeaponData && currentWeaponData->size() > 1) // Ensure there's always at least one weapon
						{
							int& selectedWeaponIndex = settings.isPrimaryActive ?
								settings.characters[settings.selectedCharacterIndex].selectedweapon[0] :
								settings.characters[settings.selectedCharacterIndex].selectedweapon[1];

							currentWeaponData->erase(currentWeaponData->begin() + selectedWeaponIndex);
							selectedWeaponIndex = std::max(0, static_cast<int>(currentWeaponData->size()) - 1);
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						else
						{
							// Optionally, show a message to the user
							ImGui::OpenPopup(xorstr_("Cannot Remove Weapon"));
						}
					}

					if (ImGui::BeginPopupModal(xorstr_("Cannot Remove Weapon"), NULL, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::Text(xorstr_("Cannot remove the last weapon."));
						if (ImGui::Button(xorstr_("OK"), ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
						ImGui::EndPopup();
					}

					if (currentWeaponData && !currentWeaponData->empty())
					{
						int& selectedWeaponIndex = settings.isPrimaryActive ?
							settings.characters[settings.selectedCharacterIndex].selectedweapon[0] :
							settings.characters[settings.selectedCharacterIndex].selectedweapon[1];

						selectedWeaponIndex = std::min(selectedWeaponIndex, static_cast<int>(currentWeaponData->size()) - 1);

						auto& weapon = (*currentWeaponData)[selectedWeaponIndex];

						char weaponNameBuffer[256];
						strncpy(weaponNameBuffer, weapon.weaponname.c_str(), sizeof(weaponNameBuffer));
						weaponNameBuffer[sizeof(weaponNameBuffer) - 1] = '\0';

						if (ImGui::InputText(xorstr_("Weapon Name"), weaponNameBuffer, sizeof(weaponNameBuffer)))
						{
							if (strlen(weaponNameBuffer) > 0)
							{
								weapon.weaponname = weaponNameBuffer;
								globals.filesystem.unsavedChanges = true;
							}
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Weapon Data"));

						if (ImGui::Checkbox(xorstr_("Rapid Fire"), &weapon.rapidfire))
						{
							globals.filesystem.unsavedChanges = true;
							settings.weaponDataChanged = true;
						}

						if (settings.game == xorstr_("Siege"))
						{
							ImGui::Text(xorstr_("Attachments"));

							const char* Sights[] = { xorstr_("1x"), xorstr_("2.5x"), xorstr_("3.5x") };
							const char* Grips[] = { xorstr_("Horizontal/Angled/None"), xorstr_("Vertical") };
							const char* Barrels[] = { xorstr_("Suppressor/Extended/None"), xorstr_("Muzzle Break (Semi-Auto Only)"), xorstr_("Compensator"), xorstr_("Flash Hider") };

							// Ensure the attachment vector has the correct size
							if (weapon.attachments.size() < 3)
								weapon.attachments.resize(3, 0);

							if (ImGui::Combo(xorstr_("Sight"), &weapon.attachments[0], Sights, IM_ARRAYSIZE(Sights)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Grip"), &weapon.attachments[1], Grips, IM_ARRAYSIZE(Grips)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Barrel"), &weapon.attachments[2], Barrels, IM_ARRAYSIZE(Barrels)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
						}
						else if (settings.game == xorstr_("Rust"))
						{
							ImGui::Text(xorstr_("Attachments"));

							const char* Sights[] = { xorstr_("None"), xorstr_("Handmade"), xorstr_("Holosight"), xorstr_("8x Scope"), xorstr_("16x Scope") };
							const char* Grips[] = { xorstr_("None"), xorstr_("Gas Compression Overdrive") };
							const char* Barrels[] = { xorstr_("None/Suppressor"), xorstr_("Muzzle Break"), xorstr_("Muzzle Boost") };

							// Ensure the attachment vector has the correct size
							if (weapon.attachments.size() < 3)
								weapon.attachments.resize(3, 0);

							if (ImGui::Combo(xorstr_("Sight"), &weapon.attachments[0], Sights, IM_ARRAYSIZE(Sights)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Grip"), &weapon.attachments[1], Grips, IM_ARRAYSIZE(Grips)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							if (ImGui::Combo(xorstr_("Barrel"), &weapon.attachments[2], Barrels, IM_ARRAYSIZE(Barrels)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
						}
					}
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::SeparatorText(xorstr_("Recoil Pattern Management"));

				ImGui::Checkbox(xorstr_("Edit Recoil Pattern"), &editingPattern);

				if (editingPattern)
				{
					std::vector<weaponData>* currentWeaponData = nullptr;
					if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust") || settings.game == xorstr_("Overwatch"))
					{
						if (!settings.characters.empty())
						{
							currentWeaponData = &settings.characters[0].weapondata;
						}
					}
					else if (settings.selectedCharacterIndex < settings.characters.size())
					{
						currentWeaponData = &settings.characters[settings.selectedCharacterIndex].weapondata;
					}

					if (currentWeaponData && !currentWeaponData->empty())
					{
						int selectedWeaponIndex = settings.isPrimaryActive ?
							settings.characters[settings.selectedCharacterIndex].selectedweapon[0] :
							settings.characters[settings.selectedCharacterIndex].selectedweapon[1];

						selectedWeaponIndex = std::min(selectedWeaponIndex, static_cast<int>(currentWeaponData->size()) - 1);

						auto& weapon = (*currentWeaponData)[selectedWeaponIndex];

						if (ImGui::Button(xorstr_("Add Point")))
						{
							weapon.values.push_back({ 0.0f, 0.0f, 1.0f });
							globals.filesystem.unsavedChanges = true;
							settings.weaponDataChanged = true;
						}

						ImGui::BeginChild(xorstr_("PatternEditor"), ImVec2(0, 0), true);
						for (size_t i = 0; i < weapon.values.size(); i++)
						{
							ImGui::PushID(static_cast<int>(i));
							if (ImGui::InputFloat3("", weapon.values[i].data()))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							ImGui::SameLine();
							if (ImGui::Button(xorstr_("Remove")))
							{
								weapon.values.erase(weapon.values.begin() + i);
								i--;
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							ImGui::PopID();
						}
						ImGui::EndChild();
					}
				}

				ImGui::EndChild();

				ImGui::NextColumn();

				// Right column: Recoil pattern display
				ImGui::BeginChild(xorstr_("RecoilPatternDisplay"));

				std::vector<weaponData>* currentWeaponData = nullptr;
				if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust") || settings.game == xorstr_("Overwatch"))
				{
					if (!settings.characters.empty())
					{
						currentWeaponData = &settings.characters[0].weapondata;
					}
				}
				else if (settings.selectedCharacterIndex < settings.characters.size())
				{
					currentWeaponData = &settings.characters[settings.selectedCharacterIndex].weapondata;
				}

				if (currentWeaponData && !currentWeaponData->empty())
				{
					int selectedWeaponIndex = settings.isPrimaryActive ?
						settings.characters[settings.selectedCharacterIndex].selectedweapon[0] :
						settings.characters[settings.selectedCharacterIndex].selectedweapon[1];

					selectedWeaponIndex = std::min(selectedWeaponIndex, static_cast<int>(currentWeaponData->size()) - 1);

					auto& weapon = (*currentWeaponData)[selectedWeaponIndex];
					DrawRecoilPattern(weapon.values);
				}
				else
				{
					ImGui::Text(xorstr_("No weapon selected or data available."));
				}

				ImGui::EndChild();

				ImGui::Columns(1);

				ImGui::EndTabItem();
			}

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
				const bool is_selected = (globals.filesystem.activeFileIndex == i);
				if (ImGui::Selectable(globals.filesystem.configFiles[i].c_str(), is_selected, ImGuiSelectableFlags_AllowDoubleClick)) {
					globals.filesystem.activeFileIndex = i;
					if (ImGui::IsMouseDoubleClicked(0)) {
						// Double-click to load
						if (globals.filesystem.unsavedChanges) {
							loadConfigPopup = true;
						}
						else {
							globals.filesystem.activeFile = globals.filesystem.configFiles[i];
							settings.selectedCharacterIndex = 0;
							settings.weaponOffOverride = false;
							settings.readSettings(globals.filesystem.activeFile, true, true);
							globals.filesystem.unsavedChanges = false;
						}
					}
				}

				// Highlight the selected item
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
					if (globals.filesystem.unsavedChanges) {
						loadConfigPopup = true;
					}
					else {
						globals.filesystem.activeFile = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
						settings.selectedCharacterIndex = 0;
						settings.weaponOffOverride = false;
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
			ImGui::Text(xorstr_("Mode: %s"), settings.mode.c_str());
			ImGui::Text(xorstr_("Game: %s"), settings.game.c_str());
			ImGui::Text(xorstr_("Characters: %d"), settings.characters.size());
			ImGui::Text(xorstr_("Potato Mode: %s"), settings.potato ? xorstr_("Enabled") : xorstr_("Disabled"));

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

	newConfigPopup(NewConfigPopup, newConfigName, selectedMode, selectedGame);
	renameConfigPopup(RenameConfigPopup, renameBuffer);

	ImGui::End();
}