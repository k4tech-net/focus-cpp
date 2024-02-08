#include "utils.hpp"

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

std::string Utils::readTextFromFile(const char* filePath) {
	std::ifstream file(filePath);
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return content;
}

bool Utils::saveTextToFile(const char* filePath, const std::string& content) {
	std::ofstream file(filePath);
	if (!file.is_open()) {
		return false;
	}

	file << content;
	file.close();

	return true;
}

std::vector<std::string> Utils::scanCurrentDirectoryForJsonFiles() {
	std::vector<std::string> jsonFiles;
	std::filesystem::path currentPath = std::filesystem::current_path();
	for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
		if (entry.is_regular_file() && entry.path().extension() == ".json") {
			jsonFiles.push_back(entry.path().filename().string());
		}
	}
	return jsonFiles;
}

bool Utils::isEdited(const std::string& original, const std::string& changed) {
	if (original == changed) {
		return false;
	}
	else {
		return true;
	}
}

bool Utils::startUpChecksRunner() {

	//TODO: Move THIS
	HKEY hKey;
	std::wstring deviceKeyName = L"SYSTEM\\ControlSet001\\Control\\DeviceClasses\\{1abc05c0-c378-41b9-9cef-df1aba82b015}";
	std::wstring subkeyName = L"";
	std::wstring deviceLocation;

	// Open the registry key for the device classes
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, deviceKeyName.c_str(), 0, KEY_READ, &hKey);
	if (result == ERROR_SUCCESS) {
		// Enumerate subkeys to find the one containing DeviceInstance
		DWORD index = 0;
		WCHAR subkeyNameBuffer[MAX_PATH];
		DWORD subkeyNameSize = MAX_PATH;
		while (RegEnumKeyEx(hKey, index, subkeyNameBuffer, &subkeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
			subkeyName = subkeyNameBuffer;
			HKEY hSubKey;
			// Open the subkey
			result = RegOpenKeyEx(hKey, subkeyName.c_str(), 0, KEY_READ, &hSubKey);
			if (result == ERROR_SUCCESS) {
				// Check if DeviceInstance entry exists
				DWORD valueType;
				WCHAR deviceInstanceBuffer[MAX_PATH];
				DWORD deviceInstanceSize = MAX_PATH;
				result = RegQueryValueEx(hSubKey, L"DeviceInstance", NULL, &valueType, reinterpret_cast<LPBYTE>(deviceInstanceBuffer), &deviceInstanceSize);
				if (result == ERROR_SUCCESS && valueType == REG_SZ) {
					deviceLocation = deviceInstanceBuffer;
					std::wcout << "Location: " << deviceLocation << std::endl;
					RegCloseKey(hSubKey);  // Close the subkey
					break;  // Exit loop since DeviceInstance entry is found
				}
				RegCloseKey(hSubKey);  // Close the subkey
			}
			++index;
			subkeyNameSize = MAX_PATH;
		}

		// Close the registry key
		RegCloseKey(hKey);
	}
	else {
		std::cerr << "Failed to open registry key for the device classes. Error code: " << result << std::endl;
	}

	return 0;
}