#include "menu.hpp"

Settings cfg;

bool Menu::comboBoxChar(const char* label, int& characterIndex, const std::vector<Settings>& items) {
    if (ImGui::BeginCombo(label, items[characterIndex].charactername.c_str())) {
        for (int i = 0; i < items.size(); i++) {
            const bool isSelected = (characterIndex == i);
            if (ImGui::Selectable(items[i].charactername.c_str(), isSelected)) {
                characterIndex = i;

                ImGui::SetItemDefaultFocus();
                ImGui::EndCombo();
                return true; // Return true if an item is selected
            }
        }
        ImGui::EndCombo();
    }
    return false; // Return false if no item is selected
}

bool Menu::comboBoxWep(const char* label, int& characterIndex, int& weaponIndex, const std::vector<Settings>& items, bool& currautofire) {
    if (ImGui::BeginCombo(label, items[characterIndex].weapondata[weaponIndex].weaponname.c_str())) {
        for (int i = 0; i < items[characterIndex].weapondata.size(); i++) {
            const bool isSelected = (weaponIndex == i);
            if (ImGui::Selectable(items[characterIndex].weapondata[i].weaponname.c_str(), isSelected)) {
                weaponIndex = i; // Update the current index if an item is selected
                currautofire = items[characterIndex].weapondata[i].autofire;

                ImGui::SetItemDefaultFocus();
                ImGui::EndCombo();
                return true; // Return true if an item is selected
            }
        }
        ImGui::EndCombo();
    }
    return false; // Return false if no item is selected
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

    if (ImGui::BeginCombo(label, displayString.c_str())) {
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

void Menu::popup(bool trigger, const char* type) {
    if (trigger) {
		ImGui::OpenPopup(type);
    }

    if (ImGui::BeginPopupModal(type)) {
        ImGui::Text(xorstr_("You have unsaved changes. Are you sure you want to complete this action?"));
        ImGui::Separator();

        if (ImGui::Button(xorstr_("Cancel"), ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            g.initshutdown = false;
            trigger = false;
        }
        ImGui::SameLine();

        if (type == xorstr_("Open")) {
            if (ImGui::Button(xorstr_("Open Anyway"), ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                editor.SetText(ut.readTextFromFile(g.editor.jsonFiles[g.editor.activeFileIndex].c_str()));
                g.editor.activeFile = g.editor.jsonFiles[g.editor.activeFileIndex];
                g.characterinfo.jsonData = ut.readTextFromFile(g.editor.jsonFiles[g.editor.activeFileIndex].c_str());
                trigger = false;
            }
        }
        else if (type == xorstr_("InitShutdown")) {
            if (ImGui::Button(xorstr_("Close Anyway"), ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                g.done = true;
                trigger = false;
            }
        }

        ImGui::EndPopup();
    }
}

void Menu::updateCharacterData(bool updatecharacter, bool updatewpns, bool updateautofire, bool updateattachments, bool updateoptions) {
	cfg.readSettings(g.editor.activeFile.c_str(), CHI.characters, true);

	if (updatecharacter) {
		CHI.selectedCharacter = CHI.characters[CHI.selectedCharacterIndex];
	}
	if (updatewpns) {
		CHI.selectedPrimary = CHI.characters[CHI.selectedCharacterIndex].defaultweapon[0];
		CHI.selectedSecondary = CHI.characters[CHI.selectedCharacterIndex].defaultweapon[1];
	}
	if (updateautofire) {
		CHI.primaryAutofire = CHI.characters[CHI.selectedCharacterIndex].weapondata[CHI.selectedPrimary].autofire;
		CHI.secondaryAutofire = CHI.characters[CHI.selectedCharacterIndex].weapondata[CHI.selectedSecondary].autofire;
	}
	if (updateattachments) {
		CHI.primaryAttachments = CHI.characters[CHI.selectedCharacterIndex].weapondata[CHI.selectedPrimary].attachments;
		CHI.secondaryAttachments = CHI.characters[CHI.selectedCharacterIndex].weapondata[CHI.selectedSecondary].attachments;
	}
	if (updateoptions) {
		CHI.characterOptions = CHI.characters[CHI.selectedCharacterIndex].options;
	}
}

void Menu::startupchecks_gui() {
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin(xorstr_("Startup Checks"), NULL, STARTUPFLAGS);

    if (g.startup.driver) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text(xorstr_("Driver is running"));
        std::string str = ut.wstring_to_string(ms.findDriver());
        ImGui::Text(str.c_str());
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text(xorstr_("Driver is not running"));
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    if (g.startup.files) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text(xorstr_("Settings files found"));
        for (int i = 0; i < g.editor.jsonFiles.size(); i++) {
            ImGui::Text(g.editor.jsonFiles[i].c_str());
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

	if (g.startup.dxgi) {
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

    ImGui::End();
}

void weaponKeyHandler() {
	static bool weaponPressedOld = false;
	bool weaponPressed = false;

	for (const auto& keybind : CHI.wpn_keybinds) {
		if (GetAsyncKeyState(std::stoi(keybind, nullptr, 0))) {
			if (!weaponPressedOld) {
				CHI.isPrimaryActive = !CHI.isPrimaryActive;
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
		if (wParam == WM_MOUSEWHEEL && CHI.mode == xorstr_("Character")) {
			if (CHI.characterOptions[2]) {
				CHI.isPrimaryActive = !CHI.isPrimaryActive;
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
	while (!g.shutdown) {
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

	for (const auto& keybind : CHI.aux_keybinds) {
		if (GetAsyncKeyState(std::stoi(keybind, nullptr, 0))) {
			if (!weaponPressedOld) {
				CHI.weaponOffOverride = !CHI.weaponOffOverride;
				weaponPressedOld = true;
			}

			weaponPressed = true;
		}
	}

	if (!weaponPressed) {
		weaponPressedOld = false; // Reset the flag if no key was pressed
	}
}

std::vector<float> calculateSensitivityModifier() {
	float oldBaseSens = 10;
	float oldRelativeSens = 50;

	float newBaseXSens = CHI.sensitivity[0];
	float newBaseYSens = CHI.sensitivity[1];
	float new1xSens = CHI.sensitivity[2];
	float new25xSens = CHI.sensitivity[3];
	float new35xSens = CHI.sensitivity[4];

	int activescope = 0;

	float sightEffect = 1.0f;
	float gripEffect = 1.0f;
	float barrelXEffect = 1.0f;
	float barrelYEffect = 1.0f;

	if (CHI.isPrimaryActive) {
		switch (CHI.primaryAttachments[0]) {
			case 0:
				sightEffect = 1.0f;
				activescope = 1;
				break;
			case 1:
				sightEffect = 2.42f;
				activescope = 2;
				break;
			case 2:
				sightEffect = 3.5f;
				activescope = 3;
				break;
		}

		switch (CHI.primaryAttachments[1]) {
			case 0:
				gripEffect = 1.0f;
				break;
			case 1:
				gripEffect = 0.8f;
				break;
		}

		switch (CHI.primaryAttachments[2]) {
			case 0:
				barrelXEffect = 1.0f;
				barrelYEffect = 1.0f;
				break;
			case 1:
				barrelXEffect = 0.75f;
				barrelYEffect = 0.36f;
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
	else {
		switch (CHI.secondaryAttachments[0]) {
			case 0:
				sightEffect = 1.0f;
				activescope = 1;
				break;
			case 1:
				sightEffect = 2.42f;
				activescope = 2;
				break;
			case 2:
				sightEffect = 3.5f;
				activescope = 3;
				break;
		}

		switch (CHI.secondaryAttachments[1]) {
			case 0:
				gripEffect = 1.0f;
				break;
			case 1:
				gripEffect = 0.8f;
				break;
		}

		switch (CHI.secondaryAttachments[2]) {
			case 0:
				barrelXEffect = 1.0f;
				barrelYEffect = 1.0f;
				break;
			case 1:
				barrelXEffect = 0.75f;
				barrelYEffect = 0.36f;
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

	float totalAttachXEffect = sightEffect * barrelXEffect;
	float totalAttachYEffect = sightEffect * gripEffect * barrelYEffect;

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

	float newSensXModifier = ((oldBaseSens * oldRelativeSens) / (scopeModifier * newBaseXSens) * totalAttachXEffect);
	float newSensYModifier = ((oldBaseSens * oldRelativeSens) / (scopeModifier * newBaseYSens) * totalAttachYEffect);

	return std::vector<float>{ newSensXModifier, newSensYModifier };
}

// Function to parse keybinds and update global struct
void keybindManager() {

	if (CHI.mode == xorstr_("Generic")) {
		CHI.isPrimaryActive = true;
		CHI.activeWeapon = CHI.selectedCharacter.weapondata[CHI.selectedPrimary];
		CHI.currAutofire = CHI.primaryAutofire;
		CHI.activeWeaponSensXModifier = 1.f;
		CHI.activeWeaponSensYModifier = 1.f;
	}
	else if (CHI.mode == xorstr_("Character")) {

		if (CHI.characterOptions[0]) {

			cv::Mat src = dx.CaptureDesktopDXGI();
			if (!src.empty()) {
				dx.detectWeaponR6(src, 25, 75);

				#if _DEBUG
				imshow("output", src); // Debug window
				#endif
			}
		}

		if (CHI.characterOptions[1]) {
			weaponKeyHandler();
		}

		// Scrolling options is handled in its thread

		if (CHI.characterOptions[3]) {
			auxKeyHandler();
		}

		CHI.activeWeaponSensXModifier = 1.f;
		CHI.activeWeaponSensYModifier = 1.f;

		if (CHI.isPrimaryActive) {
			CHI.activeWeapon = CHI.selectedCharacter.weapondata[CHI.selectedPrimary];
			CHI.currAutofire = CHI.primaryAutofire;
		}
		else {
			CHI.activeWeapon = CHI.selectedCharacter.weapondata[CHI.selectedSecondary];
			CHI.currAutofire = CHI.secondaryAutofire;
		}
	}
	else if (CHI.mode == xorstr_("Game")) {
		if (CHI.game == xorstr_("Siege")) {
			if (CHI.characterOptions[0]) {

				cv::Mat src = dx.CaptureDesktopDXGI();
				if (!src.empty()) {
					dx.detectWeaponR6(src, 25, 75);

					#if _DEBUG
					imshow("output", src); // Debug window
					#endif
				}
			}

			std::vector<float> sens = calculateSensitivityModifier();

			CHI.activeWeaponSensXModifier = sens[0];
			CHI.activeWeaponSensYModifier = sens[1];

			if (CHI.isPrimaryActive) {
				CHI.activeWeapon = CHI.selectedCharacter.weapondata[CHI.selectedPrimary];
				CHI.currAutofire = CHI.primaryAutofire;
			}
			else {
				CHI.activeWeapon = CHI.selectedCharacter.weapondata[CHI.selectedSecondary];
				CHI.currAutofire = CHI.secondaryAutofire;
			}
		}
	}
}

void Menu::gui()
{
	bool inverseShutdown = !g.initshutdown;

	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin(xorstr_("Focus"), &inverseShutdown, WINDOWFLAGS);

	auto cpos = editor.GetCursorPosition();
	bool ro = editor.IsReadOnly();

	bool openmodal = false;
	bool initshutdownpopup = false;

	g.editor.unsavedChanges = ut.isEdited(CHI.jsonData, editor.GetText());

	keybindManager();

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(xorstr_("File")))
		{
			if (ImGui::MenuItem(xorstr_("Save"), xorstr_("Ctrl-S"), nullptr, !ro))
			{
				auto textToSave = editor.GetText();
				if (ut.saveTextToFile(g.editor.activeFile.c_str(), textToSave)) {
					CHI.jsonData = ut.readTextFromFile(g.editor.activeFile.c_str());
				}

				updateCharacterData(true, true, true, true, true);
			}
			if (ImGui::MenuItem(xorstr_("Refresh"), xorstr_("Ctrl-R")))
			{
				g.editor.jsonFiles = ut.scanCurrentDirectoryForJsonFiles();
			}
			if (ImGui::BeginMenu(xorstr_("Open"))) {
				for (int i = 0; i < g.editor.jsonFiles.size(); i++) {
					if (ImGui::MenuItem(g.editor.jsonFiles[i].c_str())) {
						g.editor.activeFileIndex = i;
						if (!g.editor.unsavedChanges) {
							editor.SetText(ut.readTextFromFile(g.editor.jsonFiles[i].c_str()));
							g.editor.activeFile = g.editor.jsonFiles[g.editor.activeFileIndex];
							CHI.jsonData = ut.readTextFromFile(g.editor.jsonFiles[i].c_str());
							CHI.selectedCharacterIndex = 0;
							CHI.weaponOffOverride = false;
							updateCharacterData(true, true, true, true, true);
						}
						else {
							openmodal = true;
						}
					}
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu(xorstr_("Edit")))
		{
			if (ImGui::MenuItem(xorstr_("Read-only mode"), nullptr, &ro))
				editor.SetReadOnly(ro);
			ImGui::Separator();

			if (ImGui::MenuItem(xorstr_("Undo"), xorstr_("ALT-Backspace"), nullptr, !ro && editor.CanUndo()))
				editor.Undo();
			if (ImGui::MenuItem(xorstr_("Redo"), xorstr_("Ctrl-Y"), nullptr, !ro && editor.CanRedo()))
				editor.Redo();

			ImGui::Separator();

			if (ImGui::MenuItem(xorstr_("Copy"), xorstr_("Ctrl-C"), nullptr, editor.HasSelection()))
				editor.Copy();
			if (ImGui::MenuItem(xorstr_("Cut"), xorstr_("Ctrl-X"), nullptr, !ro && editor.HasSelection()))
				editor.Cut();
			if (ImGui::MenuItem(xorstr_("Delete"), xorstr_("Del"), nullptr, !ro && editor.HasSelection()))
				editor.Delete();
			if (ImGui::MenuItem(xorstr_("Paste"), xorstr_("Ctrl-V"), nullptr, !ro && ImGui::GetClipboardText() != nullptr))
				editor.Paste();

			ImGui::Separator();

			if (ImGui::MenuItem(xorstr_("Select all"), nullptr, nullptr))
				editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(xorstr_("View")))
		{
			if (ImGui::MenuItem(xorstr_("Dark palette")))
				editor.SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem(xorstr_("Light palette")))
				editor.SetPalette(TextEditor::GetLightPalette());
			if (ImGui::MenuItem(xorstr_("Retro blue palette")))
				editor.SetPalette(TextEditor::GetRetroBluePalette());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::BeginTabBar(xorstr_("##TabBar")))
	{
		if (CHI.mode == xorstr_("Generic")) {
			if (ImGui::BeginTabItem(xorstr_("Generic"))) {
				if (CHI.characters.size() > 0) {
					CHI.selectedCharacter = CHI.characters[CHI.selectedCharacterIndex];

					if (comboBoxWep(xorstr_("Weapon"), CHI.selectedCharacterIndex, CHI.selectedPrimary, CHI.characters, CHI.primaryAutofire)) {
						updateCharacterData(true, false, true, true, true);
					}

					ImGui::Checkbox(xorstr_("AutoFire"), &CHI.primaryAutofire);
				}
				else {
					ImGui::Text(xorstr_("Please load a weapons file"));
				}

				ImGui::EndTabItem();
			}
		}
		else if (CHI.mode == xorstr_("Character")) {
			if (ImGui::BeginTabItem(xorstr_("Character"))) {
				if (CHI.characters.size() > 0) {
					CHI.selectedCharacter = CHI.characters[CHI.selectedCharacterIndex];

					if (comboBoxChar(xorstr_("Character"), CHI.selectedCharacterIndex, CHI.characters)) {
						updateCharacterData(true, true, true, true, true);
					}

					ImGui::Spacing();
					ImGui::Spacing();

					if (CHI.weaponOffOverride) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
						ImGui::SeparatorText(xorstr_("Primary"));
						ImGui::PopStyleColor();
					}
					else if (CHI.isPrimaryActive) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						ImGui::SeparatorText(xorstr_("Primary"));
						ImGui::PopStyleColor();
					}
					else {
						ImGui::SeparatorText(xorstr_("Primary"));
					}

					if (comboBoxWep(xorstr_("Primary"), CHI.selectedCharacterIndex, CHI.selectedPrimary, CHI.characters, CHI.primaryAutofire)) {
						updateCharacterData(true, false, true, true, false);
					}
					ImGui::Checkbox(xorstr_("Primary AutoFire"), &CHI.primaryAutofire);

					ImGui::Spacing();
					ImGui::Spacing();

					if (CHI.weaponOffOverride) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
						ImGui::SeparatorText(xorstr_("Secondary"));
						ImGui::PopStyleColor();
					}
					else if (!CHI.isPrimaryActive) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						ImGui::SeparatorText(xorstr_("Secondary"));
						ImGui::PopStyleColor();
					}
					else {
						ImGui::SeparatorText(xorstr_("Secondary"));
					}

					if (comboBoxWep(xorstr_("Secondary"), CHI.selectedCharacterIndex, CHI.selectedSecondary, CHI.characters, CHI.secondaryAutofire)) {
						updateCharacterData(true, false, true, true, false);
					}
					ImGui::Checkbox(xorstr_("Secondary AutoFire"), &CHI.secondaryAutofire);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SeparatorText(xorstr_("Options"));

					std::vector<const char*> MultiOptions = { xorstr_("R6 Auto Weapon Detection"), xorstr_("Manual Weapon Detection"), xorstr_("Scroll Detection"), xorstr_("Aux Disable"), xorstr_("Gadget Detection Override") };

					if (multiCombo(xorstr_("Options"), MultiOptions, CHI.characterOptions)) {
						updateCharacterData(true, false, false, true, false);
					}
				}
				else {
					ImGui::Text(xorstr_("Please load a weapons file"));
				}

				ImGui::EndTabItem();
			}
		}
		else if (CHI.mode == xorstr_("Game")) {
			if (CHI.game == "Siege") {
				if (ImGui::BeginTabItem(xorstr_("Game"))) {
					if (CHI.characters.size() > 0) {
						ImGui::Text(xorstr_("Game: %s"), CHI.game.c_str());

						CHI.selectedCharacter = CHI.characters[CHI.selectedCharacterIndex];

						if (comboBoxChar(xorstr_("Character"), CHI.selectedCharacterIndex, CHI.characters)) {
							updateCharacterData(true, true, true, true, true);
						}

						ImGui::Spacing();
						ImGui::Spacing();

						if (CHI.weaponOffOverride) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
							ImGui::SeparatorText(xorstr_("Primary"));
							ImGui::PopStyleColor();
						}
						else if (CHI.isPrimaryActive) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
							ImGui::SeparatorText(xorstr_("Primary"));
							ImGui::PopStyleColor();
						}
						else {
							ImGui::SeparatorText(xorstr_("Primary"));
						}

						const char* Sights[] = { xorstr_("1x"), xorstr_("2.5x"), xorstr_("3.5x") };
						const char* Grips[] = { xorstr_("Horizontal/Angled/None"), xorstr_("Vertical") };
						const char* Barrels[] = { xorstr_("Supressor/Extended/None"), xorstr_("Muzzle Break (DMRs)"), xorstr_("Compensator"), xorstr_("Flash Hider") };

						if (comboBoxWep(xorstr_("Primary"), CHI.selectedCharacterIndex, CHI.selectedPrimary, CHI.characters, CHI.primaryAutofire)) {
							updateCharacterData(true, false, true, true, false);
						}
						ImGui::Checkbox(xorstr_("Primary AutoFire"), &CHI.primaryAutofire);
						if (ImGui::Combo(xorstr_("Primary Sight"), &CHI.primaryAttachments[0], Sights, 3)) {
							updateCharacterData(true, false, false, false, false);
						}
						if (ImGui::Combo(xorstr_("Primary Grip"), &CHI.primaryAttachments[1], Grips, 2)) {
							updateCharacterData(true, false, false, false, false);
						}
						if (ImGui::Combo(xorstr_("Primary Barrel"), &CHI.primaryAttachments[2], Barrels, 4)) {
							updateCharacterData(true, false, false, false, false);
						}

						ImGui::Spacing();
						ImGui::Spacing();

						if (CHI.weaponOffOverride) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
							ImGui::SeparatorText(xorstr_("Secondary"));
							ImGui::PopStyleColor();
						}
						else if (!CHI.isPrimaryActive) {
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
							ImGui::SeparatorText(xorstr_("Secondary"));
							ImGui::PopStyleColor();
						}
						else {
							ImGui::SeparatorText(xorstr_("Secondary"));
						}

						if (comboBoxWep(xorstr_("Secondary"), CHI.selectedCharacterIndex, CHI.selectedSecondary, CHI.characters, CHI.secondaryAutofire)) {
							updateCharacterData(true, false, true, true, false);

						}
						ImGui::Checkbox(xorstr_("Secondary AutoFire"), &CHI.secondaryAutofire);
						if (ImGui::Combo(xorstr_("Secondary Sight"), &CHI.secondaryAttachments[0], Sights, 3)) {
							updateCharacterData(true, false, false, false, false);
						}
						if (ImGui::Combo(xorstr_("Secondary Grip"), &CHI.secondaryAttachments[1], Grips, 2)) {
							updateCharacterData(true, false, false, false, false);
						}
						if (ImGui::Combo(xorstr_("Secondary Barrel"), &CHI.secondaryAttachments[2], Barrels, 4)) {
							updateCharacterData(true, false, false, false, false);
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Options"));

						std::vector<const char*> MultiOptions = { xorstr_("R6 Auto Weapon Detection"), xorstr_("Gadget Detection Override") };

						if (multiCombo(xorstr_("Options"), MultiOptions, CHI.characterOptions)) {
							updateCharacterData(true, false, false, false, false);
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Extra Data"));

						ImGui::SliderFloat(xorstr_("X Base Sensitivity"), &CHI.sensitivity[0], 0.0f, 100.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("Y Base Sensitivity"), &CHI.sensitivity[1], 0.0f, 100.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("1x Sensitivity"), &CHI.sensitivity[2], 0.0f, 200.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("2.5x Sensitivity"), &CHI.sensitivity[3], 0.0f, 200.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("3.5x Sensitivity"), &CHI.sensitivity[4], 0.0f, 200.0f, xorstr_("%.0f"));

						ImGui::Text(xorstr_("X Sensitivity Modifier: %f"), CHI.activeWeaponSensXModifier);
						ImGui::Text(xorstr_("Y Sensitivity Modifier: %f"), CHI.activeWeaponSensYModifier);
					}
					else {
						ImGui::Text(xorstr_("Please load a weapons file"));
					}

					ImGui::EndTabItem();
				}
			}
			else {
				ImGui::Text(xorstr_("Please load a valid game config"));
			}
		}


		if (ImGui::BeginTabItem(xorstr_("Edit"))) {

			ImGui::Text(xorstr_("%6d/%-6d %6d lines  | %s | %s | %s | %s | %s"), cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.IsOverwrite() ? xorstr_("Ovr") : xorstr_("Ins"),
				editor.CanUndo() ? xorstr_("*") : xorstr_(" "),
				g.editor.unsavedChanges ? xorstr_("*") : xorstr_(" "),
				editor.GetLanguageDefinition().mName.c_str(), g.editor.activeFile.c_str());

			ImGui::Spacing();
			ImGui::Spacing();

			editor.Render(xorstr_("TextEditor"));

			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
			{
				auto textToSave = editor.GetText();
				if (ut.saveTextToFile(g.editor.activeFile.c_str(), textToSave)) {
					CHI.jsonData = ut.readTextFromFile(g.editor.activeFile.c_str());
				}

				updateCharacterData(true, true, true, true, true);
			}

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	g.initshutdown = !inverseShutdown;

	if (g.initshutdown) {
		if (!g.editor.unsavedChanges) {
			g.done = true;
		}
		else {
			initshutdownpopup = true;
		}
	}

	popup(openmodal, xorstr_("Open"));
	popup(initshutdownpopup, xorstr_("InitShutdown"));

	ImGui::End();
}