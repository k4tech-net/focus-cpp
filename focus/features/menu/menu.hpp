#pragma once

#include "../../includes.hpp"

class Menu
{
public:
	bool comboBox(const char* label, int& currentIndex, const std::vector<Settings>& items);

	std::string readTextFromFile(const char* filePath);
	bool saveTextToFile(const char* filePath, const std::string& content);
};