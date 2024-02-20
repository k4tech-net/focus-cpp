#pragma once

#include "../../includes.hpp"

class Menu
{
public:
	bool comboBoxGen(const char* label, int& currentIndex, const std::vector<Settings>& items, bool& currautofire, int& currxdeadtime);
	bool comboBoxChar(const char* label, int& characterIndex, const std::vector<Settings>& items);
	bool comboBoxWep(const char* label, int& characterIndex, int& weaponIndex, const std::vector<Settings>& items, bool& currautofire);
	void popup(bool trigger, const char* type);
};