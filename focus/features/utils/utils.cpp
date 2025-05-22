#include "utils.hpp"

Mouse ms;
Keyboard kb;
DXGI dx;

void Utils::preciseSleep(double seconds) {
	using namespace std;
	using namespace std::chrono;

	static double estimate = 5e-3;
	static double mean = 5e-3;
	static double m2 = 0;
	static int64_t count = 1;

	while (seconds > estimate) {
		auto start = high_resolution_clock::now();
		std::this_thread::sleep_for(milliseconds(1));
		auto end = high_resolution_clock::now();

		double observed = (end - start).count() / 1e9;
		seconds -= observed;

		++count;
		double delta = observed - mean;
		mean += delta / count;
		m2 += delta * (observed - mean);
		double stddev = sqrt(m2 / (count - 1));
		estimate = mean + stddev;
	}

	// spin lock
	auto start = high_resolution_clock::now();
	while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}

void Utils::preciseSleepUntil(const std::chrono::steady_clock::time_point& targetTime) {
	using namespace std::chrono;

	auto now = steady_clock::now();
	while (now < targetTime) {
		auto remainingTime = targetTime - now;
		preciseSleep(duration_cast<duration<double>>(remainingTime).count());
		now = steady_clock::now();
	}
}

std::vector<std::string> Utils::scanCurrentDirectoryForConfigFiles() {
	std::vector<std::string> configFiles;
	std::filesystem::path currentPath = std::filesystem::current_path();
	for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
		if (entry.is_regular_file() && entry.path().extension() == xorstr_(".focus")) {
			configFiles.push_back(entry.path().filename().string());
		}
	}
	return configFiles;
}

bool Utils::isEdited(const std::string& original, const std::string& changed) {
	if (original == changed) {
		return false;
	}
	else {
		return true;
	}
}

std::string Utils::wstring_to_string(const std::wstring& wstr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
	std::string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &str[0], size_needed, nullptr, nullptr);
	return str;
}

ULONG  generateUniqueMarker() {
	std::random_device rd; // Seed for the random number generator
	std::mt19937 gen(rd());
	std::uniform_int_distribution<ULONG> dis(1, ULONG_MAX); // Exclude 0 to avoid conflicts with unmarked inputs
	return dis(gen);
}

bool Utils::initilizeMarker() {
	try {
		globals.mouseinfo.marker.store(generateUniqueMarker(), std::memory_order_relaxed);
		return true;
	}
	catch (...) {
		return false;
	}
}

int checkAVXSupport() {
	// Check if OS supports XSAVE/XRSTOR for saving AVX states
	int cpuInfo[4];
	__cpuid(cpuInfo, 1);
	bool osSupportsAVX = (cpuInfo[2] & (1 << 27)) != 0;  // OSXSAVE flag

	if (!osSupportsAVX) {
		return 0;
	}

	// Check if OS supports AVX state saving
	unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
	bool avxStateSupported = (xcrFeatureMask & 6) == 6;
	bool avx512StateSupported = (xcrFeatureMask & 0xE6) == 0xE6;

	if (!avxStateSupported) {
		return 0;
	}

	// Check CPU AVX support
	bool hasAVX = (cpuInfo[2] & (1 << 28)) != 0;  // AVX flag
	if (!hasAVX) {
		return 0;
	}

	// Check CPU AVX2 and AVX512 support
	__cpuidex(cpuInfo, 7, 0);

	if (avx512StateSupported && (cpuInfo[1] & (1 << 16))) {  // AVX-512F flag
		return 2;
	}

	if (cpuInfo[1] & (1 << 5)) {  // AVX2 flag
		return 1;
	}

	return 0;
}

void Utils::startUpChecksRunner() {
	if (ms.mouse_open()) {
		globals.startup.mouse_driver.store(true);
	}

	if (kb.keyboard_open()) {
		globals.startup.keyboard_driver.store(true);
	}

	if (globals.filesystem.configFiles.size() > 0) {
		globals.startup.files.store(true);
	}

	if (dx.InitDXGI()) {
		globals.startup.dxgi.store(true);
	}

	if (initilizeMarker()) {
		globals.startup.marker.store(true);
	}

	globals.startup.avx.store(checkAVXSupport());

	if (globals.startup.mouse_driver.load() && globals.startup.keyboard_driver.load() && globals.startup.dxgi.load() && globals.startup.marker.load()) {
		globals.startup.passedstartup.store(true);
	}
	else {
		globals.startup.passedstartup.store(false);
	}

	globals.startup.hasFinished.store(true);
	return;
}

int Utils::findCharacterIndex(const std::string& characterName) {
	for (size_t i = 0; i < settings.characters.size(); ++i) {
		if (settings.characters[i].charactername == characterName) {
			return static_cast<int>(i);
		}
	}
	return -1; // Character not found
}

int Utils::hammingDistance(const IconHash& hash1, const IconHash& hash2) {
	return (hash1 ^ hash2).count();
}

void Utils::pressMouse1(bool press) {
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dx = 0;
	input.mi.dy = 0;
	input.mi.mouseData = 0;
	input.mi.dwFlags = press ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
	input.mi.time = 0;
	input.mi.dwExtraInfo = globals.mouseinfo.marker.load(std::memory_order_relaxed);

	SendInput(1, &input, sizeof(INPUT));
}

std::string Utils::getDocumentsPath() {
    char path[MAX_PATH];
    HRESULT result = SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path);

    if (SUCCEEDED(result)) {
        return std::string(path);
    }
    else {
        // Fallback - try to get it from the environment
        const char* userProfile = std::getenv("USERPROFILE");
        if (userProfile) {
            return std::string(userProfile) + "\\Documents";
        }
    }

    // If all else fails, return empty string
    return "";
}

std::vector<std::string> Utils::findGameSettingsFiles() {
    std::vector<std::string> settingsFiles;
    std::string basePath = getDocumentsPath() + "\\My Games\\Rainbow Six - Siege\\";

    try {
        // Check if the directory exists
        if (!std::filesystem::exists(basePath)) {
            return settingsFiles;
        }

        // Iterate through all UID folders
        for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
            if (entry.is_directory()) {
                std::string potentialFile = entry.path().string() + "\\GameSettings.ini";
                if (std::filesystem::exists(potentialFile)) {
                    settingsFiles.push_back(potentialFile);
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << xorstr_("Error finding game settings: ") << e.what() << std::endl;
    }

    return settingsFiles;
}

std::string Utils::getMostRecentFile(const std::vector<std::string>& files) {
    if (files.empty()) {
        return xorstr_("");
    }

    try {
        std::string mostRecentFile = files[0];
        std::filesystem::file_time_type mostRecentTime = std::filesystem::last_write_time(files[0]);

        for (size_t i = 1; i < files.size(); ++i) {
            std::filesystem::file_time_type fileTime = std::filesystem::last_write_time(files[i]);
            if (fileTime > mostRecentTime) {
                mostRecentTime = fileTime;
                mostRecentFile = files[i];
            }
        }

        return mostRecentFile;
    }
    catch (const std::exception& e) {
        std::cerr << xorstr_("Error determining most recent file: ") << e.what() << std::endl;
        return files[0]; // Return first file as fallback
    }
}

int mapAspectRatioSiege(int aspectRatio) {
    // Get screen dimensions from the global capture variables
    int screenWidth = globals.capture.desktopWidth.load(std::memory_order_relaxed);
    int screenHeight = globals.capture.desktopHeight.load(std::memory_order_relaxed);

    // For auto (0) or resolution-based (1) settings, calculate the aspect ratio from screen dimensions
    if (aspectRatio == 0 || aspectRatio == 1) {
        // Calculate the greatest common divisor to find the simplest ratio form
        int gcd = std::gcd(screenWidth, screenHeight);
        int widthRatio = screenWidth / gcd;
        int heightRatio = screenHeight / gcd;

        // Map common aspect ratios to the application's internal indexes
        // 16:9 (most common widescreen format)
        if (widthRatio == 16 && heightRatio == 9) {
            return 0;
        }
        // 4:3 (standard/legacy format)
        else if (widthRatio == 4 && heightRatio == 3) {
            return 1;
        }
        // 5:4 format
        else if (widthRatio == 5 && heightRatio == 4) {
            return 2;
        }
        // 3:2 format
        else if (widthRatio == 3 && heightRatio == 2) {
            return 3;
        }
        // 16:10 format
        else if (widthRatio == 16 && heightRatio == 10 || widthRatio == 8 && heightRatio == 5) {
            return 4;
        }
        // 5:3 format
        else if (widthRatio == 5 && heightRatio == 3) {
            return 5;
        }
        // 19:10 format (somewhat close to 16:9)
        else if (widthRatio == 19 && heightRatio == 10) {
            return 6;
        }
        // 21:9 ultrawide format
        else if ((float)widthRatio / heightRatio >= 2.3f && (float)widthRatio / heightRatio <= 2.4f) {
            return 7;
        }
        // If no exact match, default to 16:9 (most common)
        else {
            return 0;
        }
    }

    switch (aspectRatio) {
	case 2: // 5:4
		return 2;
	case 3: // 4:3
		return 1;
	case 4: // 3:2
		return 3;
	case 5: // 16:10
		return 4;
    case 6: // 5:3
        return 5;
    case 7: // 16:9 
        return 0;
    case 8: // 19:10
        return 6;
    case 9: // 21:9
        return 7;
	default:
		return 0;
	}
}

bool Utils::parseGameSettings(const std::string& filepath, int& aspectRatio, float& fov) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    bool foundAspectRatio = false;
    bool foundFOV = false;

    std::string section;

    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';') {
            continue;
        }

        // Check for section headers [Section]
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }

        // Check for AspectRatio in DISPLAY_SETTINGS section
        if (section == xorstr_("DISPLAY_SETTINGS")) {
            if (line.find(xorstr_("AspectRatio=")) == 0) {
                aspectRatio = mapAspectRatioSiege(std::stoi(line.substr(12)));
                foundAspectRatio = true;
            }
            else if (line.find(xorstr_("DefaultFOV=")) == 0) {
                fov = std::stof(line.substr(11));
                foundFOV = true;
            }
        }

        // If we found both settings, we can stop reading the file
        if (foundAspectRatio && foundFOV) {
            break;
        }
    }

    file.close();
    return (foundAspectRatio && foundFOV);
}

bool Utils::parseSensitivitySettings(const std::string& filepath, std::vector<float>& sensitivity) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    std::string section;
    bool foundYaw = false;
	bool foundPitch = false;
    bool foundXFactor = false;
    bool foundSpecific = false;
    bool found1x = false;
    bool found25x = false;
    bool found35x = false;

    // Initialize sensitivity vector with 6 elements
    sensitivity.resize(6, 0.0f);

    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == ';') {
            continue;
        }

        // Check for section headers [Section]
        if (line[0] == '[' && line[line.length() - 1] == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }

        // Check for sensitivity settings in INPUT section
        if (section == xorstr_("INPUT")) {
            if (line.find(xorstr_("MouseYawSensitivity=")) == 0) {
                sensitivity[0] = std::stof(line.substr(20)); // Yaw
                foundYaw = true;
            } 
            else if (line.find(xorstr_("MousePitchSensitivity=")) == 0) {
				sensitivity[1] = std::stof(line.substr(22));
				foundPitch = true;
			}
            else if (line.find(xorstr_("ADSMouseUseSpecific=")) == 0) {
			    foundSpecific = std::stof(line.substr(20));
			}
            else if (!foundSpecific && line.find(xorstr_("ADSMouseSensitivityGlobal=")) == 0) {
                sensitivity[2] = std::stof(line.substr(26)); // 1x
                sensitivity[3] = std::stof(line.substr(26)); // 2.5x
				sensitivity[4] = std::stof(line.substr(26)); // 3.5x
				found1x = true;
				found25x = true;
				found35x = true;
            }
            else if (foundSpecific && line.find(xorstr_("ADSMouseSensitivity1x=")) == 0) {
                sensitivity[2] = std::stof(line.substr(22)); // 1x
                found1x = true;
            }
            else if (foundSpecific && line.find(xorstr_("ADSMouseSensitivity2xHalf=")) == 0) {
                sensitivity[3] = std::stof(line.substr(26)); // 2.5x
                found25x = true;
            }
            else if (foundSpecific && line.find(xorstr_("ADSMouseSensitivity4x=")) == 0) {
                sensitivity[4] = std::stof(line.substr(22)); // 3.5x
                found35x = true;
            }
            else if (line.find(xorstr_("MouseSensitivityMultiplierUnit=")) == 0) {
                sensitivity[5] = std::stof(line.substr(32)); // Multiplier
                foundXFactor = true;
            }
        }
    }

    file.close();
    return (foundYaw && foundPitch && foundXFactor && found1x && found25x && found35x);
}

bool Utils::applySiegeSettings() {
    // Find all potential GameSettings.ini files
    std::vector<std::string> settingsFiles = findGameSettingsFiles();
    if (settingsFiles.empty()) {
        std::cerr << xorstr_("No GameSettings.ini files found") << std::endl;
        return false;
    }

    // Get the most recent file
    std::string mostRecentFile = getMostRecentFile(settingsFiles);
    std::cout << xorstr_("Using settings from: ") << mostRecentFile << std::endl;

    // Parse the file to extract settings
    int aspectRatio;
    float fov;
    if (!parseGameSettings(mostRecentFile, aspectRatio, fov)) {
        std::cerr << xorstr_("Failed to parse game settings") << std::endl;
        return false;
    }

    // Parse sensitivity settings
    std::vector<float> sensitivity;
    bool sensSuccess = parseSensitivitySettings(mostRecentFile, sensitivity);

    // Apply the settings to our application
    settings.globalSettings.aspect_ratio = aspectRatio;
    settings.globalSettings.fov = fov;

    // Apply sensitivity settings if they were successfully parsed
    if (sensSuccess) {
        settings.globalSettings.sensitivity = sensitivity;
    }

    // Mark settings as changed
    settings.activeState.weaponDataChanged = true;
    globals.filesystem.unsavedChanges.store(true);

    return true;
}