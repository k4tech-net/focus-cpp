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

void Menu::popup(bool trigger, int type) {

	const char* chartype;

	if (type == 0) {
		chartype = xorstr_("Open");
	}
	else if (type == 1) {
		chartype = xorstr_("InitShutdown");
	}

    if (trigger) {
		ImGui::OpenPopup(chartype);
    }

    if (ImGui::BeginPopupModal(chartype)) {
        ImGui::Text(xorstr_("You have unsaved changes. Are you sure you want to complete this action?"));
        ImGui::Separator();

        if (ImGui::Button(xorstr_("Cancel"), ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            globals.initshutdown = false;
            trigger = false;
        }
        ImGui::SameLine();

        if (type == 0) {
            if (ImGui::Button(xorstr_("Open Anyway"), ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
				globals.filesystem.activeFile = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
                trigger = false;
            }
        }
        else if (type == 1) {
            if (ImGui::Button(xorstr_("Close Anyway"), ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
				globals.done = true;
                trigger = false;
            }
        }

        ImGui::EndPopup();
    }
}

void Menu::startupchecks_gui() {
    ImGui::SetNextWindowSize(ImVec2(350, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin(xorstr_("Startup Checks"), NULL, STARTUPFLAGS);

    if (globals.startup.driver) {
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

	if (settings.isPrimaryActive) {
		switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[0]) {
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

		switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[1]) {
			case 0:
				gripEffect = 1.0f;
				break;
			case 1:
				gripEffect = 0.8f;
				break;
		}

		switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[2]) {
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
	else {
		switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[0]) {
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

		switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[1]) {
			case 0:
				gripEffect = 1.0f;
				break;
			case 1:
				gripEffect = 0.8f;
				break;
		}

		switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[2]) {
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

	switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[0]) {
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

	switch (settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[2]) {
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

	std::vector<float> x, y;
	float maxX = 0, maxY = 0;
	float currentX = 0, currentY = 0;

	x.push_back(currentX);
	y.push_back(currentY);

	for (const auto& point : recoilData) {
		if (point.size() >= 3) {
			float dx = point[0];
			float dy = point[1];
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

	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	#if !_DEBUG
	ImGui::Begin(xorstr_("Focus"), &inverseShutdown, WINDOWFLAGS);
	#else
	ImGui::Begin(xorstr_("Focus DEBUG"), &inverseShutdown, WINDOWFLAGS);
	#endif

	bool openmodal = false;
	bool initshutdownpopup = false;

	//g.filesystem.unsavedChanges = ut.isEdited(g.characterinfo.jsonData, editor.GetText());

	keybindManager();

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu(xorstr_("File")))
		{
			if (ImGui::MenuItem(xorstr_("Save")))
			{
				settings.saveSettings(globals.filesystem.activeFile);

				settings.readSettings(globals.filesystem.activeFile, false, false);
			}
			if (ImGui::MenuItem(xorstr_("Convert Configs")))
			{
				std::vector<std::string> jsonFiles = ut.scanCurrentDirectoryForJsonFiles();

				// Create a vector to store successfully converted files
				std::vector<std::string> convertedFiles;

				// Iterate through all JSON files
				for (const auto& jsonFile : jsonFiles)
				{
					std::string textFilename = jsonFile.substr(0, jsonFile.find_last_of('.')) + ".txt";

					try {
						cfg.convertJsonToTextConfig(jsonFile, textFilename);
						convertedFiles.push_back(jsonFile);
					}
					catch (const std::exception& e) {
						std::cerr << "Error converting " << jsonFile << ": " << e.what() << std::endl;
					}
				}

				globals.filesystem.configFiles = ut.scanCurrentDirectoryForConfigFiles();
			}
			if (ImGui::MenuItem(xorstr_("Refresh"), xorstr_("Ctrl-R")))
			{
				globals.filesystem.configFiles = ut.scanCurrentDirectoryForConfigFiles();
			}
			if (ImGui::BeginMenu(xorstr_("Open"))) {
				for (int i = 0; i < globals.filesystem.configFiles.size(); i++) {
					if (ImGui::MenuItem(globals.filesystem.configFiles[i].c_str())) {
						globals.filesystem.activeFileIndex = i;
						if (!globals.filesystem.unsavedChanges) {
							globals.filesystem.activeFile = globals.filesystem.configFiles[globals.filesystem.activeFileIndex];
							settings.selectedCharacterIndex = 0;
							settings.weaponOffOverride = false;
							settings.readSettings(globals.filesystem.activeFile, true, true);
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

		ImGui::EndMenuBar();
	}

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

					ImGui::Checkbox(xorstr_("AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire);
					ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato);
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
					}
					ImGui::Checkbox(xorstr_("Primary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire);

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
					}
					ImGui::Checkbox(xorstr_("Secondary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].autofire);

					ImGui::Spacing();
					ImGui::Spacing();
					ImGui::SeparatorText(xorstr_("Options"));

					std::vector<const char*> MultiOptions = { xorstr_("R6 Auto Weapon Detection"), xorstr_("Manual Weapon Detection"), xorstr_("Scroll Detection"), xorstr_("Aux Disable"), xorstr_("Gadget Detection Override") };

					if (multiCombo(xorstr_("Options"), MultiOptions, settings.characters[settings.selectedCharacterIndex].options)) {
						settings.weaponDataChanged = true;
					}

					ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato);
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
						}
						ImGui::Checkbox(xorstr_("Primary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire);
						if (comboBoxGen(xorstr_("Primary Sight"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[0], Sights, 3)) {
							settings.weaponDataChanged = true;
						}
						if (comboBoxGen(xorstr_("Primary Grip"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[1], Grips, 2)) {
							settings.weaponDataChanged = true;
						}
						if (comboBoxGen(xorstr_("Primary Barrel"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[2], Barrels, 4)) {
							settings.weaponDataChanged = true;
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

						}
						ImGui::Checkbox(xorstr_("Secondary AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].autofire);
						if (comboBoxGen(xorstr_("Secondary Sight"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[0], Sights, 3)) {
							settings.weaponDataChanged = true;
						}
						if (comboBoxGen(xorstr_("Secondary Grip"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[1], Grips, 2)) {
							settings.weaponDataChanged = true;
						}
						if (comboBoxGen(xorstr_("Secondary Barrel"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]].attachments[2], Barrels, 4)) {
							settings.weaponDataChanged = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Options"));

						std::vector<const char*> MultiOptions = { xorstr_("R6 Auto Weapon Detection"), xorstr_("Gadget Detection Override") };

						if (multiCombo(xorstr_("Options"), MultiOptions, settings.characters[settings.selectedCharacterIndex].options)) {
							settings.weaponDataChanged = true;
						}

						ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato);

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Extra Data"));

						ImGui::SliderFloat(xorstr_("X Base Sensitivity"), &settings.sensitivity[0], 0.0f, 100.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("Y Base Sensitivity"), &settings.sensitivity[1], 0.0f, 100.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("1x Sensitivity"), &settings.sensitivity[2], 0.0f, 200.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("2.5x Sensitivity"), &settings.sensitivity[3], 0.0f, 200.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("3.5x Sensitivity"), &settings.sensitivity[4], 0.0f, 200.0f, xorstr_("%.0f"));
						ImGui::SliderFloat(xorstr_("Sensitivity Multiplier"), &settings.sensitivity[5], 0.0f, 1.0f, xorstr_("%.3f"));

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
						ImGui::Checkbox(xorstr_("AutoFire"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].autofire);
						if (comboBoxGen(xorstr_("Sight"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[0], Sights, 5)) {
							settings.weaponDataChanged = true;
						}
						if (comboBoxGen(xorstr_("Barrel"), &settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].attachments[2], Barrels, 3)) {
							settings.weaponDataChanged = true;
						}

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Options"));

						std::vector<const char*> MultiOptions = { xorstr_("Rust Auto Weapon Detection") };

						if (multiCombo(xorstr_("Options"), MultiOptions, settings.characters[settings.selectedCharacterIndex].options)) {
							settings.weaponDataChanged = true;
						}

						ImGui::Checkbox(xorstr_("Potato Mode"), &settings.potato);

						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::SeparatorText(xorstr_("Extra Data"));

						ImGui::SliderFloat(xorstr_("Sensitivity"), &settings.sensitivity[0], 0.0f, 10.0f, xorstr_("%.3f"));
						ImGui::SliderFloat(xorstr_("Aiming Sensitivity"), &settings.sensitivity[1], 0.0f, 10.0f, xorstr_("%.3f"));

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

		if (settings.mode == xorstr_("Character") || settings.game == xorstr_("Siege") || settings.game == xorstr_("Rust") || settings.game == xorstr_("Generic")) {
			if (ImGui::BeginTabItem(xorstr_("Aim"))) {

				if (!settings.aimbotData.enabled) {
					std::vector<const char*> providers = { xorstr_("CPU"), xorstr_("CUDA") };
					ImGui::Combo(xorstr_("Provider"), &settings.aimbotData.provider, providers.data(), (int)providers.size());
				}

				ImGui::Checkbox(xorstr_("AI Aim Assist"), &settings.aimbotData.enabled);

				if (settings.aimbotData.enabled) {
					ImGui::SliderInt(xorstr_("Aim Assist Smoothing"), &settings.aimbotData.smoothing, 1, 200);
					ImGui::SliderInt(xorstr_("Max Distance per Tick"), &settings.aimbotData.maxDistance, 1, 100);
					ImGui::SliderFloat(xorstr_("% of Total Distance"), &settings.aimbotData.percentDistance, 0.01f, 1.0f, xorstr_("%.2f"));

					std::vector<const char*> AimbotHitbox = { xorstr_("Body"), xorstr_("Head"), xorstr_("Closest") };
					ImGui::Combo(xorstr_("Hitbox"), &settings.aimbotData.hitbox, AimbotHitbox.data(), (int)AimbotHitbox.size());
				}

				ImGui::EndTabItem();
			}

			

			//if (ImGui::BeginTabItem(xorstr_("Config Editor")))
			//{
			//	static int selectedCharacterIndex = 0;
			//	static int selectedWeaponIndex = 0;
			//	static char characterNameBuffer[256] = "";
			//	static char weaponNameBuffer[256] = "";

			//	ImGui::Columns(2);

			//	// Left column: Character and Weapon selection
			//	ImGui::BeginChild(xorstr_("Left Column"));

			//	if (settings.mode == xorstr_("Character") || settings.game == xorstr_("Siege"))
			//	{
			//		if (ImGui::Button(xorstr_("Add Character")))
			//		{
			//			settings.characters.push_back(Settings());
			//			settings.characters.back().charactername = xorstr_("New Character");
			//		}

			//		if (g.characterinfo.characters.size() > 0)
			//		{
			//			if (ImGui::ListBox(xorstr_("Characters"), &selectedCharacterIndex,
			//				[](void* data, int idx, const char** out_text) {
			//					std::vector<Settings>* characters = (std::vector<Settings>*)data;
			//					*out_text = characters->at(idx).charactername.c_str();
			//					return true;
			//				}, &g.characterinfo.characters, g.characterinfo.characters.size()))
			//			{
			//				selectedWeaponIndex = 0;
			//				strncpy(characterNameBuffer, g.characterinfo.characters[selectedCharacterIndex].charactername.c_str(), sizeof(characterNameBuffer));
			//				characterNameBuffer[sizeof(characterNameBuffer) - 1] = '\0';
			//			}

			//			if (ImGui::InputText(xorstr_("Character Name"), characterNameBuffer, sizeof(characterNameBuffer)))
			//			{
			//				g.characterinfo.characters[selectedCharacterIndex].charactername = characterNameBuffer;
			//			}

			//			if (ImGui::Button(xorstr_("Remove Character")))
			//			{
			//				if (selectedCharacterIndex < g.characterinfo.characters.size())
			//				{
			//					g.characterinfo.characters.erase(g.characterinfo.characters.begin() + selectedCharacterIndex);
			//					selectedCharacterIndex = std::max(0, selectedCharacterIndex - 1);
			//				}
			//			}

			//			ImGui::Separator();
			//		}
			//	}

			//	if (ImGui::Button(xorstr_("Add Weapon")))
			//	{
			//		if (g.characterinfo.mode == xorstr_("Generic") || g.characterinfo.game == xorstr_("Rust"))
			//		{
			//			if (g.characterinfo.characters.empty())
			//			{
			//				g.characterinfo.characters.push_back(Settings());
			//			}
			//			g.characterinfo.characters[0].weapondata.push_back(weaponData());
			//			g.characterinfo.characters[0].weapondata.back().weaponname = xorstr_("New Weapon");
			//		}
			//		else if (selectedCharacterIndex < g.characterinfo.characters.size())
			//		{
			//			g.characterinfo.characters[selectedCharacterIndex].weapondata.push_back(weaponData());
			//			g.characterinfo.characters[selectedCharacterIndex].weapondata.back().weaponname = xorstr_("New Weapon");
			//		}
			//	}

			//	std::vector<weaponData>* currentWeaponData = nullptr;
			//	if (g.characterinfo.mode == xorstr_("Generic") || g.characterinfo.game == xorstr_("Rust"))
			//	{
			//		if (!g.characterinfo.characters.empty())
			//		{
			//			currentWeaponData = &g.characterinfo.characters[0].weapondata;
			//		}
			//	}
			//	else if (selectedCharacterIndex < g.characterinfo.characters.size())
			//	{
			//		currentWeaponData = &g.characterinfo.characters[selectedCharacterIndex].weapondata;
			//	}

			//	if (currentWeaponData && !currentWeaponData->empty())
			//	{
			//		if (ImGui::ListBox(xorstr_("Weapons"), &selectedWeaponIndex,
			//			[](void* data, int idx, const char** out_text) {
			//				std::vector<weaponData>* weapons = (std::vector<weaponData>*)data;
			//				*out_text = weapons->at(idx).weaponname.c_str();
			//				return true;
			//			}, currentWeaponData, currentWeaponData->size()))
			//		{
			//			strncpy(weaponNameBuffer, (*currentWeaponData)[selectedWeaponIndex].weaponname.c_str(), sizeof(weaponNameBuffer));
			//			weaponNameBuffer[sizeof(weaponNameBuffer) - 1] = '\0';
			//		}

			//		if (ImGui::InputText(xorstr_("Weapon Name"), weaponNameBuffer, sizeof(weaponNameBuffer)))
			//		{
			//			(*currentWeaponData)[selectedWeaponIndex].weaponname = weaponNameBuffer;
			//		}

			//		if (ImGui::Button(xorstr_("Remove Weapon")))
			//		{
			//			if (selectedWeaponIndex < currentWeaponData->size())
			//			{
			//				currentWeaponData->erase(currentWeaponData->begin() + selectedWeaponIndex);
			//				selectedWeaponIndex = std::max(0, selectedWeaponIndex - 1);
			//			}
			//		}
			//	}

			//	ImGui::EndChild();

			//	ImGui::NextColumn();

			//	// Right column: Weapon details and pattern editor
			//	ImGui::BeginChild(xorstr_("Right Column"));

			//	if (currentWeaponData && selectedWeaponIndex < currentWeaponData->size())
			//	{
			//		auto& weapon = (*currentWeaponData)[selectedWeaponIndex];

			//		ImGui::Checkbox(xorstr_("Auto Fire"), &weapon.autofire);

			//		if (g.characterinfo.game == xorstr_("Siege"))
			//		{
			//			ImGui::Text(xorstr_("Attachments"));

			//			const char* Sights[] = { xorstr_("1x"), xorstr_("2.5x"), xorstr_("3.5x") };
			//			const char* Grips[] = { xorstr_("Horizontal/Angled/None"), xorstr_("Vertical") };
			//			const char* Barrels[] = { xorstr_("Supressor/Extended/None"), xorstr_("Muzzle Break (Semi-Auto Only)"), xorstr_("Compensator"), xorstr_("Flash Hider") };

			//			int sightIndex = weapon.attachments[0];
			//			int gripIndex = weapon.attachments[1];
			//			int barrelIndex = weapon.attachments[2];

			//			if (ImGui::Combo(xorstr_("Sight"), &sightIndex, Sights, IM_ARRAYSIZE(Sights)))
			//			{
			//				weapon.attachments[0] = sightIndex;
			//			}
			//			if (ImGui::Combo(xorstr_("Grip"), &gripIndex, Grips, IM_ARRAYSIZE(Grips)))
			//			{
			//				weapon.attachments[1] = gripIndex;
			//			}
			//			if (ImGui::Combo(xorstr_("Barrel"), &barrelIndex, Barrels, IM_ARRAYSIZE(Barrels)))
			//			{
			//				weapon.attachments[2] = barrelIndex;
			//			}
			//		}
			//		else if (g.characterinfo.game == xorstr_("Rust"))
			//		{
			//			ImGui::Text(xorstr_("Attachments"));

			//			const char* Sights[] = { xorstr_("None"), xorstr_("Handmade"), xorstr_("Holosight"), xorstr_("8x Scope"), xorstr_("16x Scope") };
			//			const char* Barrels[] = { xorstr_("None/Suppressor"), xorstr_("Muzzle Break"), xorstr_("Muzzle Boost") };

			//			int sightIndex = weapon.attachments[0];
			//			int barrelIndex = weapon.attachments[2];

			//			if (ImGui::Combo(xorstr_("Sight"), &sightIndex, Sights, IM_ARRAYSIZE(Sights)))
			//			{
			//				weapon.attachments[0] = sightIndex;
			//			}
			//			if (ImGui::Combo(xorstr_("Barrel"), &barrelIndex, Barrels, IM_ARRAYSIZE(Barrels)))
			//			{
			//				weapon.attachments[2] = barrelIndex;
			//			}
			//		}

			//		ImGui::Separator();
			//		ImGui::Text(xorstr_("Recoil Pattern"));

			//		static bool editingPattern = false;
			//		static std::vector<std::vector<float>> tempPattern;

			//		if (ImGui::Button(editingPattern ? xorstr_("Save Pattern") : xorstr_("Edit Pattern")))
			//		{
			//			if (editingPattern)
			//			{
			//				weapon.values = tempPattern;
			//				editingPattern = false;
			//			}
			//			else
			//			{
			//				tempPattern = weapon.values;
			//				editingPattern = true;
			//			}
			//		}

			//		if (editingPattern)
			//		{
			//			if (ImGui::Button(xorstr_("Add Point")))
			//			{
			//				tempPattern.push_back({ 0.0f, 0.0f, 1.0f });
			//			}

			//			for (size_t i = 0; i < tempPattern.size(); i++)
			//			{
			//				ImGui::PushID(static_cast<int>(i));
			//				ImGui::InputFloat3(xorstr_(""), tempPattern[i].data());
			//				ImGui::SameLine();
			//				if (ImGui::Button(xorstr_("Remove")))
			//				{
			//					tempPattern.erase(tempPattern.begin() + i);
			//					i--;
			//				}
			//				ImGui::PopID();
			//			}
			//		}
			//		else
			//		{
			//			DrawRecoilPattern(weapon.values);
			//		}
			//	}

			//	ImGui::EndChild();

			//	ImGui::Columns(1);

			//	ImGui::EndTabItem();
			//}
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

	popup(openmodal, 0);
	popup(initshutdownpopup, 1);

	ImGui::End();
}