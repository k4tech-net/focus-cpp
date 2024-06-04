#pragma once

#include "../../includes.hpp"

#define STARTUPFLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiViewportFlags_NoDecoration)
#define WINDOWFLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)

class Menu
{
public:
	bool comboBoxGen(const char* label, int* current_item, const char* const items[], int items_count);
	bool comboBoxChar(const char* label, int& characterIndex, const std::vector<Settings>& items);
	bool comboBoxWep(const char* label, int& characterIndex, int& weaponIndex, const std::vector<Settings>& items, bool& currautofire);
	bool multiCombo(const char* label, std::vector<const char*>& items, std::vector<bool>& selected);
	void popup(bool trigger, int type);
	
	void updateCharacterData(bool updatecharacter, bool updatewpns, bool updateautofire, bool updateattachments, bool updateoptions, bool updateAimbotInfo);
	void startupchecks_gui();
	void mouseScrollHandler();
	void gui();
};