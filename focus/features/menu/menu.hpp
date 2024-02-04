#pragma once

#include "../../includes.hpp"

class Menu
{
public:
	bool ComboBox(const char* label, int& currentIndex, const std::vector<Settings>& items);
};