#pragma once

#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include "Windows.h"
#include <iostream>

#include "../driver/mouse.hpp"
#include "../driver/keyboard.hpp"
#include "../settings/settings.hpp"
#include "../dxgi/weaponmasks.hpp"
#include "../dxgi/dxgi.hpp"

class Utils
{
public:
	void preciseSleep(double seconds);
	void preciseSleepUntil(const std::chrono::steady_clock::time_point& targetTime);
	std::vector<std::string> scanCurrentDirectoryForConfigFiles();
	std::vector<std::string> scanCurrentDirectoryForJsonFiles();
	bool isEdited(const std::string& original, const std::string& changed);
	std::string wstring_to_string(const std::wstring& wstr);
	bool initilizeMarker();
	int findCharacterIndex(const std::string& characterName);
	int hammingDistance(const IconHash& hash1, const IconHash& hash2);

	void startUpChecksRunner();
};