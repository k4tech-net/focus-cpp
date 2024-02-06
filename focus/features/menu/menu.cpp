#include "menu.hpp"

// In your Menu class implementation (menu.cpp)
bool Menu::comboBox(const char* label, int& currentIndex, const std::vector<Settings>& items) {
    if (ImGui::BeginCombo(label, items[currentIndex].name.c_str())) {
        for (int i = 0; i < items.size(); i++) {
            const bool isSelected = (currentIndex == i);
            if (ImGui::Selectable(items[i].name.c_str(), isSelected)) {
                currentIndex = i; // Update the current index if an item is selected
                ImGui::SetItemDefaultFocus();
                ImGui::EndCombo();
                return true; // Return true if an item is selected
            }
        }
        ImGui::EndCombo();
    }
    return false; // Return false if no item is selected
}

std::string Menu::readTextFromFile(const char* filePath) {
	std::ifstream file(filePath);
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	return content;
}

bool Menu::saveTextToFile(const char* filePath, const std::string& content) {
	std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

	file << content;
	file.close();

	return true;
}