#pragma once

#include "../../includes.hpp"
#include "../imgui/plugins/implot.h"

#define STARTUPFLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiViewportFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize)
#define WINDOWFLAGS (ImGuiWindowFlags_NoCollapse)

class Menu
{
public:
	bool comboBoxGen(const char* label, int* current_item, const char* const items[], int items_count);
	bool comboBoxChar(const char* label, int& characterIndex, const std::vector<characterData>& items);
	bool comboBoxWep(const char* label, int& characterIndex, int& weaponIndex, const std::vector<characterData>& items);
	bool multiCombo(const char* label, std::vector<const char*>& items, std::vector<bool>& selected);
	void popup(bool& trigger, int type);

	void startupchecks_gui();
	void mouseScrollHandler();
	void gui();
};