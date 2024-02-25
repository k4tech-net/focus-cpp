#pragma once

#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include "Windows.h"
#include <iostream>

#include "../mouse_driver/mouse.hpp"
#include "../settings/globals.hpp"

#include "../dxgi/dxgi.hpp"

class Utils
{
public:
	void preciseSleep(double seconds);
	std::string readTextFromFile(const char* filePath);
	bool saveTextToFile(const char* filePath, const std::string& content);
	std::vector<std::string> scanCurrentDirectoryForJsonFiles();
	bool isEdited(const std::string& original, const std::string& changed);
	std::string wstring_to_string(const std::wstring& wstr);

	void startUpChecksRunner();
};