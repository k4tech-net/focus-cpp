#include "utils.hpp"

Mouse ms;
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
		if (entry.is_regular_file() && entry.path().extension() == xorstr_(".json")) {
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

std::string Utils::wstring_to_string(const std::wstring& wstr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
	std::string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], static_cast<int>(wstr.size()), &str[0], size_needed, nullptr, nullptr);
	return str;
}

void Utils::startUpChecksRunner() {
	if (ms.mouse_open()) {
		g.startup.driver = true;
	}
	else {
		g.startup.passedstartup = false;
		g.startup.hasFinished = true;
		return;
	}

	if (g.editor.jsonFiles.size() > 0) {
		g.startup.files = true;
	}

	if (dx.InitDXGI()) {
		g.startup.dxgi = true;
	}
	else {
		g.startup.passedstartup = false;
		g.startup.hasFinished = true;
		return;
	}
	
	g.startup.hasFinished = true;
	g.startup.passedstartup = true;
	return;
}