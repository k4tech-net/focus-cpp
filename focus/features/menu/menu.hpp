#pragma once

#include "../../includes.hpp"

#define STARTUPFLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)
#define WINDOWFLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)

class Menu
{
public:
	bool comboBoxGen(const char* label, int& currentIndex, const std::vector<Settings>& items, bool& currautofire, int& currxdeadtime);
	bool comboBoxChar(const char* label, int& characterIndex, const std::vector<Settings>& items);
	bool comboBoxWep(const char* label, int& characterIndex, int& weaponIndex, const std::vector<Settings>& items, bool& currautofire);
	bool multiCombo(const char* label, std::vector<const char*>& items, std::vector<bool>& selected);
	void popup(bool trigger, const char* type);
	
	void readGlobalSettings();
	void updateCharacterData();
	void startupchecks_gui();
	void mouseScrollHandler();
	void gui();
};