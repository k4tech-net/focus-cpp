#include "menu.hpp"

// In your Menu class implementation (menu.cpp)
bool Menu::comboBoxGen(const char* label, int& currentIndex, const std::vector<Settings>& items, bool& currautofire, int& currxdeadtime) {
    //if (ImGui::BeginCombo(label, items[currentIndex].name.c_str())) {
    //    for (int i = 0; i < items.size(); i++) {
    //        const bool isSelected = (currentIndex == i);
    //        if (ImGui::Selectable(items[i].name.c_str(), isSelected)) {
    //            currentIndex = i; // Update the current index if an item is selected
    //            currautofire = items[i].autofire;
				//currxdeadtime = items[i].xdeadtime;

    //            ImGui::SetItemDefaultFocus();
    //            ImGui::EndCombo();
    //            return true; // Return true if an item is selected
    //        }
    //    }
    //    ImGui::EndCombo();
    //}
    //return false; // Return false if no item is selected
    return false;
}

bool Menu::comboBoxChar(const char* label, int& characterIndex, const std::vector<Settings>& items) {
    if (ImGui::BeginCombo(label, items[characterIndex].charactername.c_str())) {
        for (int i = 0; i < items.size(); i++) {
            const bool isSelected = (characterIndex == i);
            if (ImGui::Selectable(items[i].charactername.c_str(), isSelected)) {
                characterIndex = i;

                ImGui::SetItemDefaultFocus();
                ImGui::EndCombo();
                return true; // Return true if an item is selected
            }
        }
        ImGui::EndCombo();
    }
    return false; // Return false if no item is selected
}

bool Menu::comboBoxWep(const char* label, int& characterIndex, int& weaponIndex, const std::vector<Settings>& items, bool& currautofire) {
    if (ImGui::BeginCombo(label, items[characterIndex].weapondata[weaponIndex].weaponname.c_str())) {
        for (int i = 0; i < items[characterIndex].weapondata.size(); i++) {
            const bool isSelected = (weaponIndex == i);
            if (ImGui::Selectable(items[characterIndex].weapondata[i].weaponname.c_str(), isSelected)) {
                weaponIndex = i; // Update the current index if an item is selected
                currautofire = items[characterIndex].weapondata[i].autofire;

                ImGui::SetItemDefaultFocus();
                ImGui::EndCombo();
                return true; // Return true if an item is selected
            }
        }
        ImGui::EndCombo();
    }
    return false; // Return false if no item is selected
}

bool Menu::multiCombo(const char* label, std::vector<const char*>& items, std::vector<bool>& selected) {
    bool changed = false;
    std::string displayString;
    for (int i = 0; i < items.size(); ++i) {
        if (selected[i]) {
            if (!displayString.empty())
                displayString += ", ";
            displayString += items[i];
        }
    }

    if (ImGui::BeginCombo(label, displayString.c_str())) {
        for (int i = 0; i < items.size(); ++i) {
            ImGui::PushID(i);
            bool isSelected = selected[i];
            if (ImGui::Selectable(items[i], &isSelected)) {
                selected[i] = !selected[i];
                changed = true;
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }

    return changed;
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
                g.characterinfo.jsonData = ut.readTextFromFile(g.editor.jsonFiles[g.editor.activeFileIndex].c_str());
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