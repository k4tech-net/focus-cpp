#pragma once

#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include "Windows.h"
#include <iostream>

class Utils
{
public:
	void preciseSleep(double seconds);
	std::string readTextFromFile(const char* filePath);
	bool saveTextToFile(const char* filePath, const std::string& content);
	std::vector<std::string> scanCurrentDirectoryForJsonFiles();
	bool isEdited(const std::string& original, const std::string& changed);

	bool startUpChecksRunner();
};