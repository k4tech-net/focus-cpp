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

void newConfigPopup(bool trigger, char* newConfigName, int& selectedMode, int& selectedGame) {
	if (trigger) {
		ImGui::OpenPopup("NewConfigPopup");
	}

	if (ImGui::BeginPopup("NewConfigPopup")) {
		ImGui::InputText("Config Name", newConfigName, 256);

		const char* modes[] = { "Generic", "Character", "Game" };
		ImGui::Combo("Mode", &selectedMode, modes, IM_ARRAYSIZE(modes));

		if (selectedMode == 2) {  // If "Game" mode is selected
			const char* games[] = { "Siege", "Rust" };
			ImGui::Combo("Game", &selectedGame, games, IM_ARRAYSIZE(games));
		}

		if (ImGui::Button("Create")) {
			std::string newFileName = std::string(newConfigName) + ".focus";
			Settings newSettings;
			newSettings.mode = modes[selectedMode];
			newSettings.potato = false;
			newSettings.aimbotData = { 0, 0, false, 0, 50, 10, 0.5f, 0 }; // Default aimbot settings

			// Pre-fill with example values based on mode and game
			if (newSettings.mode == "Generic") {
				characterData genericChar;
				weaponData exampleWeapon;
				exampleWeapon.weaponname = "Example Weapon";
				exampleWeapon.autofire = true;
				exampleWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
				genericChar.weapondata.push_back(exampleWeapon);
				genericChar.selectedweapon = { 0, 0 };
				newSettings.characters.push_back(genericChar);
			}
			else if (newSettings.mode == "Character") {
				newSettings.wpn_keybinds = { "0x31", "0x32" }; // Example keybinds (1 and 2 keys)
				newSettings.aux_keybinds = { "0x58" }; // Example aux keybind (X key)

				characterData exampleChar;
				exampleChar.charactername = "Example Character";
				exampleChar.options = { true, false, true, false, false };

				weaponData primaryWeapon;
				primaryWeapon.weaponname = "Primary Weapon";
				primaryWeapon.autofire = true;
				primaryWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
				exampleChar.weapondata.push_back(primaryWeapon);

				weaponData secondaryWeapon;
				secondaryWeapon.weaponname = "Secondary Weapon";
				secondaryWeapon.autofire = false;
				secondaryWeapon.values = { {0.05f, 0.05f, 1.0f}, {0.1f, 0.1f, 1.0f} };
				exampleChar.weapondata.push_back(secondaryWeapon);

				exampleChar.selectedweapon = { 0, 1 }; // Primary and secondary weapon indices
				newSettings.characters.push_back(exampleChar);
			}
			else if (newSettings.mode == "Game") {
				newSettings.game = selectedGame == 0 ? "Siege" : "Rust";

				if (newSettings.game == "Siege") {
					characterData exampleChar;
					exampleChar.charactername = "Example Operator";
					exampleChar.options = { true, false };

					weaponData primaryWeapon;
					primaryWeapon.weaponname = "Primary Weapon";
					primaryWeapon.autofire = true;
					primaryWeapon.attachments = { 0, 0, 0 };
					primaryWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
					exampleChar.weapondata.push_back(primaryWeapon);

					weaponData secondaryWeapon;
					secondaryWeapon.weaponname = "Secondary Weapon";
					secondaryWeapon.autofire = false;
					secondaryWeapon.attachments = { 0, 0, 0 };
					secondaryWeapon.values = { {0.05f, 0.05f, 1.0f}, {0.1f, 0.1f, 1.0f} };
					exampleChar.weapondata.push_back(secondaryWeapon);

					exampleChar.selectedweapon = { 0, 1 };
					newSettings.characters.push_back(exampleChar);
					newSettings.sensitivity = { 50.0f, 50.0f, 100.0f, 100.0f, 100.0f, 0.02f };
				}
				else if (newSettings.game == "Rust") {
					characterData rustChar;
					rustChar.options = { true };

					weaponData exampleWeapon;
					exampleWeapon.weaponname = "Example Weapon";
					exampleWeapon.autofire = true;
					exampleWeapon.attachments = { 0, 0, 0 };
					exampleWeapon.values = { {0.1f, 0.1f, 1.0f}, {0.2f, 0.2f, 1.0f} };
					rustChar.weapondata.push_back(exampleWeapon);

					rustChar.selectedweapon = { 0, 0 };
					newSettings.characters.push_back(rustChar);
					newSettings.sensitivity = { 0.5f, 1.0f };
					newSettings.crouch_keybind = "0x11";  // Default to Ctrl key
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
		if (ImGui::Button("Cancel")) {
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
		ImGui::OpenPopup("RenamePopup");
	}

	if (ImGui::BeginPopupModal("RenamePopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Rename config:");
		ImGui::Text("%s", globals.filesystem.configFiles[globals.filesystem.activeFileIndex].c_str());
		ImGui::InputText("New Name", renameBuffer, IM_ARRAYSIZE(renameBuffer));
		ImGui::Separator();

		if (ImGui::Button("Rename", ImVec2(120, 0))) {
			std::string oldName = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
			std::string newName = std::string(renameBuffer) + ".focus";
			if (rename(oldName.c_str(), newName.c_str()) == 0) {
				globals.filesystem.configFiles[globals.filesystem.activeFileIndex] = newName;
				if (globals.filesystem.activeFile == oldName) {
					globals.filesystem.activeFile = newName;
				}
			}
			else {
				ImGui::OpenPopup("RnameErrorPopup");
			}
			memset(renameBuffer, 0, sizeof(renameBuffer));
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
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
			ImGui::Text("You have unsaved changes in the current config.");
			ImGui::Text("Loading a new config will discard these changes.");
		}
		else if (type == 1) {
			ImGui::Text("You are about to save over a different config file.");
			ImGui::Text("Current active config: %s", globals.filesystem.activeFile.c_str());
			ImGui::Text("Selected config to save over: %s", globals.filesystem.configFiles[globals.filesystem.activeFileIndex].c_str());
		}
		else if (type == 2) {
			ImGui::Text("You are about to initiate a shutdown with pending changes.");
			ImGui::Text("Doing so will discard these changes.");
		}
		else if (type == 3) {
			ImGui::Text("You are about to delete a config file.");
			ImGui::Text("Selected config to delete: %s", globals.filesystem.configFiles[globals.filesystem.activeFileIndex].c_str());
			ImGui::Text("This action cannot be undone.");
		}

		ImGui::Separator();
		ImGui::Text("Are you sure you want to proceed?");

		if (ImGui::Button("Yes", ImVec2(120, 0))) {
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
					globals.filesystem.activeFile = "";
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
		if (ImGui::Button("No", ImVec2(120, 0))) {
			globals.initshutdown = false;
			trigger = false;
			ImGui::CloseCurrentPopup();
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

	float newBaseXSens = settings.sensitivity[0];
	float newBaseYSens = settings.sensitivity[1];
	float new1xSens = settings.sensitivity[2];
	float new25xSens = settings.sensitivity[3];
	float new35xSens = settings.sensitivity[4];
	float newMultiplier = settings.sensitivity[5];

	int activescope = 0;

	float sightXEffect = 1.0f;
	float sightYEffect = 1.0f;

	float gripEffect = 1.0f;

	float barrelXEffect = 1.0f;
	float barrelYEffect = 1.0f;

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

	float newSensXModifier = ((oldBaseSens * oldRelativeSens * oldMultiplier) / (scopeModifier * newBaseXSens * newMultiplier) * totalAttachXEffect);
	float newSensYModifier = ((oldBaseSens * oldRelativeSens * oldMultiplier) / (scopeModifier * newBaseYSens * newMultiplier) * totalAttachYEffect);

	return std::vector<float>{ newSensXModifier, newSensYModifier };
}

std::vector<float> calculateSensitivityModifierRust() {
	float oldBaseSens = 0.509f;
	float oldADSSens = 1;

	float newBaseSens = settings.sensitivity[0];
	float newADSSens = settings.sensitivity[1];

	float sightXEffect = 1.0f;
	float sightYEffect = 1.0f;

	float barrelXEffect = 1.0f;
	float barrelYEffect = 1.0f;

	if (settings.selectedCharacterIndex < settings.characters.size() &&
		!settings.characters[settings.selectedCharacterIndex].weapondata.empty()) {
		int weaponIndex = settings.characters[settings.selectedCharacterIndex].selectedweapon[0];
		weaponIndex = std::min(weaponIndex, static_cast<int>(settings.characters[settings.selectedCharacterIndex].weapondata.size()) - 1);

		const auto& weapon = settings.characters[settings.selectedCharacterIndex].weapondata[weaponIndex];

		if (weapon.attachments.size() >= 3) {
			switch (weapon.attachments[0]) {
			case 0:
				sightXEffect = 1.0f;
				sightYEffect = 1.0f;
				break;
			case 1:
				sightXEffect = 0.8f;
				sightYEffect = 0.8f;
				break;
			case 2:
				sightXEffect = 1.25f;
				sightYEffect = 1.25f;
				break;
			case 3:
				sightXEffect = 7.25f;
				sightYEffect = 7.25f;
				break;
			case 4:
				sightXEffect = 14.5f;
				sightYEffect = 14.5f;
				break;
			}

			switch (weapon.attachments[2]) {
			case 0:
				barrelXEffect = 1.0f;
				barrelYEffect = 1.0f;
				break;
			case 1:
				barrelXEffect = 0.5f;
				barrelYEffect = 0.5f;
				break;
			case 2:
				barrelXEffect = 1.1f;
				barrelYEffect = 1.1f;
				break;
			}
		}
	}

	float totalAttachXEffect = sightXEffect * barrelXEffect;
	float totalAttachYEffect = sightYEffect * barrelYEffect;

	float standingModifier = 1.f;

	if (GetAsyncKeyState(std::stoi(settings.crouch_keybind, nullptr, 0))) {
		standingModifier = 0.5f;
	}

	float newSensXModifier = ((oldBaseSens * oldADSSens) / (newBaseSens * newADSSens) * totalAttachXEffect * standingModifier);
	float newSensYModifier = ((oldBaseSens * oldADSSens) / (newBaseSens * newADSSens) * totalAttachYEffect * standingModifier);

	return std::vector<float>{ newSensXModifier, newSensYModifier };
}

// Function to parse keybinds and update global struct
void keybindManager() {

	if (settings.mode == xorstr_("Generic")) {
		settings.isPrimaryActive = true;
		settings.sensMultiplier = { 1.f, 1.f };
	}
	else if (settings.mode == xorstr_("Character")) {

		if (settings.characters[settings.selectedCharacterIndex].options[0]) {

			if (!globals.desktopMat.empty()) {
				int screenWidth = globals.desktopMat.cols;
				int screenHeight = globals.desktopMat.rows;

				// Define ratios for crop region
				float cropRatioX = 0.8f; // 20% from left
				float cropRatioY = 0.82f; // 20% from top
				float cropRatioWidth = 0.18f; // 18% of total width
				float cropRatioHeight = 0.14f; // 14% of total height

				// Calculate the region of interest (ROI) based on ratios
				int x = static_cast<int>(cropRatioX * screenWidth);
				int y = static_cast<int>(cropRatioY * screenHeight);
				int width = static_cast<int>(cropRatioWidth * screenWidth);
				int height = static_cast<int>(cropRatioHeight * screenHeight);

				// Ensure the ROI is within the bounds of the desktopMat
				x = std::max(0, x);
				y = std::max(0, y);
				width = std::min(width, screenWidth - x);
				height = std::min(height, screenHeight - y);

				cv::Rect roi(x, y, width, height);

				// Extract the region of interest from the desktopMat
				globals.desktopMutex_.lock();
				cv::Mat smallRegion = globals.desktopMat(roi);
				globals.desktopMutex_.unlock();

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

				if (!globals.desktopMat.empty()) {
					int screenWidth = globals.desktopMat.cols;
					int screenHeight = globals.desktopMat.rows;

					// Define ratios for crop region
					float cropRatioX = 0.8f; // 20% from left
					float cropRatioY = 0.82f; // 20% from top
					float cropRatioWidth = 0.18f; // 18% of total width
					float cropRatioHeight = 0.14f; // 14% of total height

					// Calculate the region of interest (ROI) based on ratios
					int x = static_cast<int>(cropRatioX * screenWidth);
					int y = static_cast<int>(cropRatioY * screenHeight);
					int width = static_cast<int>(cropRatioWidth * screenWidth);
					int height = static_cast<int>(cropRatioHeight * screenHeight);

					// Ensure the ROI is within the bounds of the desktopMat
					x = std::max(0, x);
					y = std::max(0, y);
					width = std::min(width, screenWidth - x);
					height = std::min(height, screenHeight - y);

					cv::Rect roi(x, y, width, height);

					// Extract the region of interest from the desktopMat
					globals.desktopMutex_.lock();
					cv::Mat smallRegion = globals.desktopMat(roi);
					globals.desktopMutex_.unlock();

					dx.detectWeaponR6(smallRegion, 25, 75);
				}
			}
			else {
				settings.weaponOffOverride = false;
				settings.isPrimaryActive = true;
			}

			settings.sensMultiplier = calculateSensitivityModifierR6();
		}
		else if (settings.game == xorstr_("Rust")) {
			static bool initializeRustDetector = false;

			if (settings.characters[settings.selectedCharacterIndex].options[0]) {
				if (!globals.desktopMat.empty()) {
					int screenWidth = globals.desktopMat.cols;
					int screenHeight = globals.desktopMat.rows;

					// Define ratios for crop region
					float cropRatioX = 0.3475f;
					float cropRatioY = 0.892f;
					float cropRatioWidth = 0.295f;
					float cropRatioHeight = 0.084f;

					// Calculate the region of interest (ROI) based on ratios
					int x = static_cast<int>(cropRatioX * screenWidth);
					int y = static_cast<int>(cropRatioY * screenHeight);
					int width = static_cast<int>(cropRatioWidth * screenWidth);
					int height = static_cast<int>(cropRatioHeight * screenHeight);

					// Ensure the ROI is within the bounds of the desktopMat
					x = std::max(0, x);
					y = std::max(0, y);
					width = std::min(width, screenWidth - x);
					height = std::min(height, screenHeight - y);

					cv::Rect roi(x, y, width, height);
					
					// Extract the region of interest from the desktopMat
					globals.desktopMutex_.lock();
					cv::Mat smallRegion = globals.desktopMat(roi);
					globals.desktopMutex_.unlock();

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

	if (ImPlot::BeginPlot("Recoil Pattern", ImVec2(-1, -1))) {
		ImPlot::SetupAxes("Horizontal", "Vertical", ImPlotAxisFlags_RangeFit, ImPlotAxisFlags_RangeFit | ImPlotAxisFlags_Invert);
		ImPlot::SetupAxisLimits(ImAxis_X1, -maxX, maxX, ImGuiCond_Always);
		ImPlot::SetupAxisLimits(ImAxis_Y1, -maxY * 0.1, maxY * 1.1, ImGuiCond_Always);

		// Plot the recoil pattern
		ImPlot::PlotLine("Recoil", x.data(), y.data(), x.size());

		// Plot points to show each recoil step
		ImPlot::PlotScatter("Steps", x.data(), y.data(), x.size());

		// Add labels for start and end points
		ImPlot::PlotText("Start", x[0], y[0], ImVec2(0, -10));
		ImPlot::PlotText("End", x.back(), y.back(), ImVec2(0, 10));

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

					if (ImGui::Checkbox(xorstr_("AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire)) {
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
					ImGui::Text("No weapon selected or data available.");
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
					
					if (ImGui::Checkbox(xorstr_("Primary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire)) {
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

					if (ImGui::Checkbox(xorstr_("Secondary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].autofire)) {
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
					ImGui::Text("No weapon selected or data available.");
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
						if (ImGui::Checkbox(xorstr_("Primary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
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
						if (ImGui::Checkbox(xorstr_("Secondary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].autofire)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
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

						std::vector<const char*> MultiOptions = { xorstr_("R6 Auto Weapon Detection"), xorstr_("Gadget Detection Override") };

						if (multiCombo(xorstr_("Options"), MultiOptions, settings.characters[settings.selectedCharacterIndex].options)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}

						if (ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato)) {
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Extra Data"));

						if (ImGui::SliderFloat(xorstr_("X Base Sensitivity"), &settings.sensitivity[0], 0.0f, 100.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						if (ImGui::SliderFloat(xorstr_("Y Base Sensitivity"), &settings.sensitivity[1], 0.0f, 100.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						if (ImGui::SliderFloat(xorstr_("1x Sensitivity"), &settings.sensitivity[2], 0.0f, 200.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						if (ImGui::SliderFloat(xorstr_("2.5x Sensitivity"), &settings.sensitivity[3], 0.0f, 200.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
						if (ImGui::SliderFloat(xorstr_("3.5x Sensitivity"), &settings.sensitivity[4], 0.0f, 200.0f, xorstr_("%.0f"))) {
							globals.filesystem.unsavedChanges = true;
						}
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
						ImGui::Text("No weapon selected or data available.");
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
						const char* Barrels[] = { xorstr_("None/Suppressor"), xorstr_("Muzzle Break"), xorstr_("Muzzle Boost") };

						if (comboBoxWep(xorstr_("Weapon"), settings.selectedCharacterIndex, settings.characters[settings.selectedCharacterIndex].selectedweapon[0], settings.characters)) {
							settings.weaponDataChanged = true;
						}
						if (ImGui::Checkbox(xorstr_("AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire)) {
							settings.weaponDataChanged = true;
							globals.filesystem.unsavedChanges = true;
						}
						if (comboBoxGen(xorstr_("Sight"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[0], Sights, 5)) {
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
						ImGui::Text("No weapon selected or data available.");
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

		if (settings.mode == xorstr_("Character") || settings.game == xorstr_("Siege") || settings.game == xorstr_("Rust") || settings.mode == xorstr_("Generic")) {
			if (ImGui::BeginTabItem(xorstr_("Aim"))) {

				if (!settings.aimbotData.enabled) {
					std::vector<const char*> providers = { xorstr_("CPU"), xorstr_("CUDA") };
					if (ImGui::Combo(xorstr_("Provider"), &settings.aimbotData.provider, providers.data(), (int)providers.size())) {
						globals.filesystem.unsavedChanges = true;
					}
				}

				if (ImGui::Checkbox(xorstr_("AI Aim Assist"), &settings.aimbotData.enabled)) {
					globals.filesystem.unsavedChanges = true;
				}

				if (settings.aimbotData.enabled) {
					if (ImGui::SliderInt(xorstr_("Aim Assist Smoothing"), &settings.aimbotData.smoothing, 1, 200)) {
						globals.filesystem.unsavedChanges = true;
					}
					if (ImGui::SliderInt(xorstr_("Max Distance per Tick"), &settings.aimbotData.maxDistance, 1, 100)) {
						globals.filesystem.unsavedChanges = true;
					}
					if (ImGui::SliderFloat(xorstr_("% of Total Distance"), &settings.aimbotData.percentDistance, 0.01f, 1.0f, xorstr_("%.2f"))) {
						globals.filesystem.unsavedChanges = true;
					}

					std::vector<const char*> AimbotHitbox = { xorstr_("Body"), xorstr_("Head"), xorstr_("Closest") };
					if (ImGui::Combo(xorstr_("Hitbox"), &settings.aimbotData.hitbox, AimbotHitbox.data(), (int)AimbotHitbox.size())) {
						globals.filesystem.unsavedChanges = true;
					}
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem(xorstr_("Config Editor")))
			{
				static bool editingPattern = false;
				static std::vector<std::vector<float>> tempPattern;

				ImGui::Columns(2, "ConfigEditorColumns", false);

				// Left column: All controls and editors
				ImGui::BeginChild("ControlsAndEditors");

				if (!editingPattern)
				{
					if (settings.mode == xorstr_("Character") || settings.game == xorstr_("Siege"))
					{
						ImGui::SeparatorText("Characters");

						// Character list
						ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
						ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
						ImGui::BeginChild("CharacterList", ImVec2(0, 100), true);

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

						if (ImGui::Button("Add Character"))
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

						if (ImGui::Button("Remove Character") && settings.selectedCharacterIndex < settings.characters.size())
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
								ImGui::OpenPopup("Cannot Remove Character");
							}
						}

						if (ImGui::BeginPopupModal("Cannot Remove Character", NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							ImGui::Text("Cannot remove the last character.");
							if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}

						if (settings.selectedCharacterIndex < settings.characters.size())
						{
							char characterNameBuffer[256];
							strncpy(characterNameBuffer, settings.characters[settings.selectedCharacterIndex].charactername.c_str(), sizeof(characterNameBuffer));
							characterNameBuffer[sizeof(characterNameBuffer) - 1] = '\0';

							if (ImGui::InputText("Character Name", characterNameBuffer, sizeof(characterNameBuffer)))
							{
								settings.characters[settings.selectedCharacterIndex].charactername = characterNameBuffer;
								globals.filesystem.unsavedChanges = true;
							}
						}

						ImGui::Spacing();
						ImGui::Spacing();
					}

					ImGui::SeparatorText("Weapons");

					// Weapon list
					ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
					ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
					ImGui::BeginChild("WeaponList", ImVec2(0, 100), true);

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
					std::vector<weaponData>* currentWeaponData = nullptr;
					if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust"))
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

							if (ImGui::Selectable((*currentWeaponData)[i].weaponname.c_str(), is_selected))
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

					if (ImGui::Button("Add Weapon"))
					{
						if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust"))
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

					if (ImGui::Button("Remove Weapon"))
					{
						std::vector<weaponData>* currentWeaponData = nullptr;
						if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust"))
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
							ImGui::OpenPopup("Cannot Remove Weapon");
						}
					}

					if (ImGui::BeginPopupModal("Cannot Remove Weapon", NULL, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::Text("Cannot remove the last weapon.");
						if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
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

						if (ImGui::InputText("Weapon Name", weaponNameBuffer, sizeof(weaponNameBuffer)))
						{
							weapon.weaponname = weaponNameBuffer;
							globals.filesystem.unsavedChanges = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText("Weapon Data");

						if (ImGui::Checkbox("Auto Fire", &weapon.autofire))
						{
							globals.filesystem.unsavedChanges = true;
							settings.weaponDataChanged = true;
						}

						if (settings.game == xorstr_("Siege"))
						{
							ImGui::Text("Attachments");

							const char* Sights[] = { "1x", "2.5x", "3.5x" };
							const char* Grips[] = { "Horizontal/Angled/None", "Vertical" };
							const char* Barrels[] = { "Suppressor/Extended/None", "Muzzle Break (Semi-Auto Only)", "Compensator", "Flash Hider" };

							// Ensure the attachment vector has the correct size
							if (weapon.attachments.size() < 3)
								weapon.attachments.resize(3, 0);

							if (ImGui::Combo("Sight", &weapon.attachments[0], Sights, IM_ARRAYSIZE(Sights)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							if (ImGui::Combo("Grip", &weapon.attachments[1], Grips, IM_ARRAYSIZE(Grips)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							if (ImGui::Combo("Barrel", &weapon.attachments[2], Barrels, IM_ARRAYSIZE(Barrels)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
						}
						else if (settings.game == xorstr_("Rust"))
						{
							ImGui::Text("Attachments");

							const char* Sights[] = { "None", "Handmade", "Holosight", "8x Scope", "16x Scope" };
							const char* Barrels[] = { "None/Suppressor", "Muzzle Break", "Muzzle Boost" };

							// Ensure the attachment vector has the correct size
							if (weapon.attachments.size() < 3)
								weapon.attachments.resize(3, 0);

							if (ImGui::Combo("Sight", &weapon.attachments[0], Sights, IM_ARRAYSIZE(Sights)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
							if (ImGui::Combo("Barrel", &weapon.attachments[2], Barrels, IM_ARRAYSIZE(Barrels)))
							{
								globals.filesystem.unsavedChanges = true;
								settings.weaponDataChanged = true;
							}
						}
					}
				}

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::SeparatorText("Recoil Pattern Management");

				if (ImGui::Checkbox("Edit Recoil Pattern", &editingPattern))
				{
					std::vector<weaponData>* currentWeaponData = nullptr;
					if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust"))
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
						if (editingPattern)
						{
							tempPattern = weapon.values;
						}
						else
						{
							weapon.values = tempPattern;
							globals.filesystem.unsavedChanges = true;
							settings.weaponDataChanged = true;
						}
					}
				}

				if (editingPattern)
				{
					if (ImGui::Button("Add Point"))
					{
						tempPattern.push_back({ 0.0f, 0.0f, 1.0f });
					}

					ImGui::BeginChild("PatternEditor", ImVec2(0, 0), true);
					for (size_t i = 0; i < tempPattern.size(); i++)
					{
						ImGui::PushID(static_cast<int>(i));
						if (ImGui::InputFloat3("", tempPattern[i].data()))
						{
							globals.filesystem.unsavedChanges = true;
						}
						ImGui::SameLine();
						if (ImGui::Button("Remove"))
						{
							tempPattern.erase(tempPattern.begin() + i);
							i--;
							globals.filesystem.unsavedChanges = true;
						}
						ImGui::PopID();
					}
					ImGui::EndChild();
				}

				ImGui::EndChild();

				ImGui::NextColumn();

				// Right column: Recoil pattern display
				ImGui::BeginChild("RecoilPatternDisplay");

				std::vector<weaponData>* currentWeaponData = nullptr;
				if (settings.mode == xorstr_("Generic") || settings.game == xorstr_("Rust"))
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
					DrawRecoilPattern(editingPattern ? tempPattern : weapon.values);
				}
				else
				{
					ImGui::Text("No weapon selected or data available.");
				}

				ImGui::EndChild();

				ImGui::Columns(1);

				ImGui::EndTabItem();
}

		}

		if (ImGui::BeginTabItem("Config")) {
			ImGui::Columns(2, "ConfigColumns", false);

			// Left column: Config list and file operations
			ImGui::BeginChild("ConfigList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

			if (ImGui::Button("Refresh List")) {
				globals.filesystem.configFiles = ut.scanCurrentDirectoryForConfigFiles();
			}

			ImGui::SameLine();

			if (ImGui::Button("New Config")) {
				NewConfigPopup = true;
			}

			ImGui::Separator();

			// Styled config list
			ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
			ImGui::BeginChild("ConfigList", ImVec2(0, 0), true);

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

			if (ImGui::Button("Load")) {
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

			if (ImGui::Button("Save")) {
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

			if (ImGui::Button("Delete")) {
				if (globals.filesystem.activeFileIndex >= 0 && globals.filesystem.activeFileIndex < globals.filesystem.configFiles.size()) {
					deleteConfigPopup = true;
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Rename")) {
				if (globals.filesystem.activeFileIndex >= 0 && globals.filesystem.activeFileIndex < globals.filesystem.configFiles.size()) {
					RenameConfigPopup = true;
				}
			}

			ImGui::NextColumn();

			// Right column: Config details and conversion
			ImGui::BeginChild("ConfigDetails", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

			ImGui::Text("Current Config: %s", globals.filesystem.activeFile.c_str());
			ImGui::Separator();

			ImGui::Text("Config Details:");
			ImGui::Text("Mode: %s", settings.mode.c_str());
			ImGui::Text("Game: %s", settings.game.c_str());
			ImGui::Text("Characters: %d", settings.characters.size());
			ImGui::Text("Potato Mode: %s", settings.potato ? "Enabled" : "Disabled");

			ImGui::Separator();
			static int convertedCount = 0;

			if (ImGui::Button("Convert JSON Configs")) {
				std::vector<std::string> jsonFiles = ut.scanCurrentDirectoryForJsonFiles();

				for (const auto& jsonFile : jsonFiles) {
					std::string textFilename = jsonFile.substr(0, jsonFile.find_last_of('.')) + ".focus";
					try {
						settings.convertJsonToTextConfig(jsonFile, textFilename);
						convertedCount++;
					}
					catch (const std::exception& e) {
						std::cerr << "Error converting " << jsonFile << ": " << e.what() << std::endl;
					}
				}

				ImGui::OpenPopup("ConversionResultPopup");
				globals.filesystem.configFiles = ut.scanCurrentDirectoryForConfigFiles();
			}

			if (ImGui::BeginPopupModal("ConversionResultPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Conversion complete!");
				ImGui::Text("%d files were converted successfully.", convertedCount);
				if (ImGui::Button("OK", ImVec2(120, 0))) {
					ImGui::CloseCurrentPopup();
					convertedCount = 0;
				}
				ImGui::EndPopup();
			}

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