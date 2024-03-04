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
        ImGui::Text("You have unsaved changes. Are you sure you want to complete this action?");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            g.initshutdown = false;
            trigger = false;
        }
        ImGui::SameLine();

        if (type == "Open") {
            if (ImGui::Button("Open Anyway", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                editor.SetText(ut.readTextFromFile(g.editor.jsonFiles[g.editor.activeFileIndex].c_str()));
                g.editor.activeFile = g.editor.jsonFiles[g.editor.activeFileIndex];
                g.characterinfo.jsonData = ut.readTextFromFile(g.editor.jsonFiles[g.editor.activeFileIndex].c_str());
                trigger = false;
            }
        }
        else if (type == "InitShutdown") {
            if (ImGui::Button("Close Anyway", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                g.done = true;
                trigger = false;
            }
        }

        ImGui::EndPopup();
    }
}

void Menu::readGlobalSettings() {
	auto settings = cfg.readSettings(g.editor.activeFile.c_str(), CHI.characters, true);
	CHI.mode = std::get<0>(settings);
	CHI.wpn_keybinds = std::get<1>(settings);
	CHI.aux_keybinds = std::get<2>(settings);
}

void Menu::updateCharacterData() {
	readGlobalSettings();

	if (CHI.mode == "Generic" || CHI.mode == "generic") {
		CHI.selectedCharacterIndex = 0;
	}

	CHI.selectedCharacter = CHI.characters[CHI.selectedCharacterIndex];
	CHI.selectedPrimary = CHI.characters[CHI.selectedCharacterIndex].defaultweapon[0];
	CHI.selectedSecondary = CHI.characters[CHI.selectedCharacterIndex].defaultweapon[1];
	CHI.primaryAutofire = CHI.characters[CHI.selectedCharacterIndex].weapondata[CHI.selectedPrimary].autofire;
	CHI.secondaryAutofire = CHI.characters[CHI.selectedCharacterIndex].weapondata[CHI.selectedSecondary].autofire;
	CHI.characterOptions = CHI.characters[CHI.selectedCharacterIndex].options;
}

void Menu::startupchecks_gui() {
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Startup Checks", NULL, STARTUPFLAGS);

    if (g.startup.driver) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text("Driver is running");
        std::string str = ut.wstring_to_string(ms.findDriver());
        ImGui::Text(str.c_str());
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImGui::Text("Driver is not running");
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    if (g.startup.files) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        ImGui::Text("Settings files found");
        for (int i = 0; i < g.editor.jsonFiles.size(); i++) {
            ImGui::Text(g.editor.jsonFiles[i].c_str());
        }
        ImGui::PopStyleColor();
    }
    else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.2f, 1.0f));
        ImGui::Text("No settings files found");
        ImGui::Text("Please create one and refresh from the file tab");
        ImGui::PopStyleColor();
    }

	ImGui::Separator();

	if (g.startup.dxgi) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		ImGui::Text("DXGI was initialised");
		ImGui::PopStyleColor();
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::Text("DXGI could not be initialised");
		ImGui::Text("Please contact support");
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
		if (wParam == WM_MOUSEWHEEL && (CHI.mode == "Character" || CHI.mode == "character")) {
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
		std::cerr << "Failed to set up mouse hook." << std::endl;
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
			std::cerr << "MsgWaitForMultipleObjects error" << std::endl;
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

// Function to parse keybinds and update global struct
void keybindManager() {

	if (CHI.mode == "Generic" || CHI.mode == "generic") {
		CHI.isPrimaryActive = true;
		CHI.activeWeapon = CHI.selectedCharacter.weapondata[CHI.selectedPrimary];
		CHI.currAutofire = CHI.primaryAutofire;
	}
	else if (CHI.mode == "Character" || CHI.mode == "character") {

		if (CHI.characterOptions[0]) {

			cv::Mat src = dx.CaptureDesktopDXGI();
			if (!src.empty()) {
				dx.detectWeapon(src, 0, 0);
				//imshow("output", src); // Debug window
			}
		}

		if (CHI.characterOptions[1]) {
			weaponKeyHandler();
		}

		// Scrolling options is handled in its thread

		if (CHI.characterOptions[3]) {
			auxKeyHandler();
		}

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

void Menu::gui()
{
	bool inverseShutdown = !g.initshutdown;

	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Focus", &inverseShutdown, WINDOWFLAGS);

	auto cpos = editor.GetCursorPosition();
	bool ro = editor.IsReadOnly();

	bool openmodal = false;
	bool initshutdownpopup = false;

	g.editor.unsavedChanges = ut.isEdited(CHI.jsonData, editor.GetText());

	std::vector<const char*> MultiOptions = { "R6 Auto Weapon Detection", "Manual Weapon Detection", "Scroll Detection", "Aux Disable" };

	keybindManager();

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save", "Ctrl-S", nullptr, !ro))
			{
				auto textToSave = editor.GetText();
				if (ut.saveTextToFile(g.editor.activeFile.c_str(), textToSave)) {
					CHI.jsonData = ut.readTextFromFile(g.editor.activeFile.c_str());
				}

				updateCharacterData();
			}
			if (ImGui::MenuItem("Refresh", "Ctrl-R"))
			{
				g.editor.jsonFiles = ut.scanCurrentDirectoryForJsonFiles();
			}
			if (ImGui::BeginMenu("Open")) {
				for (int i = 0; i < g.editor.jsonFiles.size(); i++) {
					if (ImGui::MenuItem(g.editor.jsonFiles[i].c_str())) {
						g.editor.activeFileIndex = i;
						if (!g.editor.unsavedChanges) {
							editor.SetText(ut.readTextFromFile(g.editor.jsonFiles[i].c_str()));
							g.editor.activeFile = g.editor.jsonFiles[g.editor.activeFileIndex];
							CHI.jsonData = ut.readTextFromFile(g.editor.jsonFiles[i].c_str());
							updateCharacterData();
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
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
				editor.SetReadOnly(ro);
			ImGui::Separator();

			if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
				editor.Undo();
			if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
				editor.Redo();

			ImGui::Separator();

			if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
				editor.Copy();
			if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
				editor.Cut();
			if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
				editor.Delete();
			if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
				editor.Paste();

			ImGui::Separator();

			if (ImGui::MenuItem("Select all", nullptr, nullptr))
				editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Dark palette"))
				editor.SetPalette(TextEditor::GetDarkPalette());
			if (ImGui::MenuItem("Light palette"))
				editor.SetPalette(TextEditor::GetLightPalette());
			if (ImGui::MenuItem("Retro blue palette"))
				editor.SetPalette(TextEditor::GetRetroBluePalette());
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (ImGui::BeginTabBar("##TabBar"))
	{
		if (CHI.mode == "Generic" || CHI.mode == "generic") {
			if (ImGui::BeginTabItem("Weapon")) {
				if (CHI.characters.size() > 0) {
					CHI.selectedCharacter = CHI.characters[CHI.selectedCharacterIndex];

					if (comboBoxWep("Weapon", CHI.selectedCharacterIndex, CHI.selectedPrimary, CHI.characters, CHI.primaryAutofire)) {
						readGlobalSettings();
					}

					ImGui::Checkbox("AutoFire", &CHI.primaryAutofire);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SeparatorText("Extra Weapon Data");

					ImGui::Text("XDeadTime: %d", CHI.activeWeapon.xdeadtime);
				}
				else {
					ImGui::Text("Please load a weapons file");
				}

				ImGui::EndTabItem();
			}
		}
		else if (CHI.mode == "Character" || CHI.mode == "character") {
			if (ImGui::BeginTabItem("Weapon")) {
				if (CHI.characters.size() > 0) {
					CHI.selectedCharacter = CHI.characters[CHI.selectedCharacterIndex];

					if (comboBoxChar("Character", CHI.selectedCharacterIndex, CHI.characters)) {
						updateCharacterData();
					}

					ImGui::Spacing();
					ImGui::Spacing();

					if (CHI.weaponOffOverride) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
						ImGui::SeparatorText("Primary");
						ImGui::PopStyleColor();
					}
					else if (CHI.isPrimaryActive) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						ImGui::SeparatorText("Primary");
						ImGui::PopStyleColor();
					}
					else {
						ImGui::SeparatorText("Primary");
					}

					if (comboBoxWep("Primary", CHI.selectedCharacterIndex, CHI.selectedPrimary, CHI.characters, CHI.primaryAutofire)) {
						readGlobalSettings();
					}
					ImGui::Checkbox("Primary AutoFire", &CHI.primaryAutofire);

					ImGui::Spacing();
					ImGui::Spacing();

					if (CHI.weaponOffOverride) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
						ImGui::SeparatorText("Secondary");
						ImGui::PopStyleColor();
					}
					else if (!CHI.isPrimaryActive) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
						ImGui::SeparatorText("Secondary");
						ImGui::PopStyleColor();
					}
					else {
						ImGui::SeparatorText("Secondary");
					}

					if (comboBoxWep("Secondary", CHI.selectedCharacterIndex, CHI.selectedSecondary, CHI.characters, CHI.secondaryAutofire)) {
						readGlobalSettings();
					}
					ImGui::Checkbox("Secondary AutoFire", &CHI.secondaryAutofire);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SeparatorText("Options");

					if (multiCombo("Options", MultiOptions, CHI.characterOptions)) {
						readGlobalSettings();
					}

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SeparatorText("Extra Weapon Data");

					ImGui::Text("XDeadTime: %d", CHI.activeWeapon.xdeadtime);
				}
				else {
					ImGui::Text("Please load a weapons file");
				}

				ImGui::EndTabItem();
			}
		}


		if (ImGui::BeginTabItem("Edit")) {

			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.IsOverwrite() ? "Ovr" : "Ins",
				editor.CanUndo() ? "*" : " ",
				g.editor.unsavedChanges ? "*" : " ",
				editor.GetLanguageDefinition().mName.c_str(), g.editor.activeFile.c_str());

			ImGui::Spacing();
			ImGui::Spacing();

			editor.Render("TextEditor");

			ImGuiIO& io = ImGui::GetIO();
			if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
			{
				auto textToSave = editor.GetText();
				if (ut.saveTextToFile(g.editor.activeFile.c_str(), textToSave)) {
					CHI.jsonData = ut.readTextFromFile(g.editor.activeFile.c_str());
				}

				updateCharacterData();
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

	popup(openmodal, "Open");
	popup(initshutdownpopup, "InitShutdown");

	ImGui::End();
}