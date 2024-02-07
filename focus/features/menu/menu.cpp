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

void Menu::popup(bool trigger, const char* type) {
    if (trigger) {
		ImGui::OpenPopup(type);
    }

    if (ImGui::BeginPopupModal(type)) {
        ImGui::Text("You have unsaved changes. Are you sure you want to complete this action?");
        ImGui::Separator();

        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            g.initshutdown = false;
            trigger = false;
        }
        ImGui::SameLine();

        if (type == "Open") {
            if (ImGui::Button("Open Anyway", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                editor.SetText(ut.readTextFromFile(g.editor.jsonFiles[g.editor.activeFileIndex].c_str()));
                g.editor.activeFile = g.editor.jsonFiles[g.editor.activeFileIndex];
                g.weaponsText = ut.readTextFromFile(g.editor.jsonFiles[g.editor.activeFileIndex].c_str());
                trigger = false;
            }
        }
        else if (type == "InitShutdown") {
            if (ImGui::Button("Close Anyway", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
                glfwSetWindowShouldClose(g.window, true);
                trigger = false;
            }
        }

        ImGui::EndPopup();
    }
}