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
		globals.startup.mouse_driver = true;
	}

	if (kb.keyboard_open()) {
		globals.startup.keyboard_driver = true;
	}

	if (globals.filesystem.configFiles.size() > 0) {
		globals.startup.files = true;
	}

	if (dx.InitDXGI()) {
		globals.startup.dxgi = true;
	}

	if (initilizeMarker()) {
		globals.startup.marker = true;
	}

	globals.startup.avx = checkAVXSupport();

	if (globals.startup.mouse_driver && globals.startup.keyboard_driver && globals.startup.dxgi && globals.startup.marker) {
		globals.startup.passedstartup = true;
	}
	else {
		globals.startup.passedstartup = false;
	}

	globals.startup.hasFinished = true;
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

