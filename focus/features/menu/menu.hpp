#pragma once

#include "../../includes.hpp"

class Menu
{
public:
	bool comboBox(const char* label, int& currentIndex, const std::vector<Settings>& items, bool& currautofire, int& currxdeadtime);
	void popup(bool trigger, const char* type);
};