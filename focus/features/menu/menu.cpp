#include "menu.hpp"

// In your Menu class implementation (menu.cpp)
bool Menu::ComboBox(const char* label, int& currentIndex, const std::vector<Settings>& items) {
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