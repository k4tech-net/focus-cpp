#include <atomic>
#include <stdio.h>
#include <xorstr.hpp>

#include "imgui.h"


enum class HotkeyMode {
    Off,
    Hold,
    Toggle,
    HoldOff,
    Always
};

enum class InputType {
    None,
    VirtualKey,  // Changed to use VirtualKey instead of separate keyboard/mouse
};

struct Hotkey {
    std::atomic<InputType> type{ InputType::None };
    std::atomic<int> vKey{ 0 };  // Store the virtual key code directly
    std::atomic<HotkeyMode> mode{ HotkeyMode::Hold };
    std::atomic<bool> state{ false };
    std::atomic<bool> isPressed{ false };
};

enum class HotkeyIndex {
    AutoQuickPeek,
    FakeSpinBot,
    AutoHashomPeek,
    COUNT
};

class HotkeySystem {
private:
    friend class Settings;
    Hotkey hotkeys[static_cast<size_t>(HotkeyIndex::COUNT)];

    bool IsKeyPressed(const Hotkey& hotkey) {
        if (hotkey.type == InputType::VirtualKey) {
            return (GetAsyncKeyState(hotkey.vKey.load()) & 0x8000) && !(GetAsyncKeyState(hotkey.vKey.load()) & 0x4000);
        }
        return false;
    }

    bool IsKeyDown(const Hotkey& hotkey) {
        if (hotkey.type == InputType::VirtualKey) {
            return GetAsyncKeyState(hotkey.vKey.load()) & 0x8000;
        }
        return false;
    }

    // Get descriptive name for virtual key codes
    const char* GetKeyName(const Hotkey& hotkey) {
        static char keyName[256];
        if (hotkey.type == InputType::None) {
            return xorstr_("None");
        }

        int vKey = hotkey.vKey.load();

        switch (vKey) {
            // Mouse buttons
        case VK_LBUTTON: return xorstr_("Mouse 1");
        case VK_RBUTTON: return xorstr_("Mouse 2");
        case VK_MBUTTON: return xorstr_("Mouse 3");
        case VK_XBUTTON1: return xorstr_("Mouse 4");
        case VK_XBUTTON2: return xorstr_("Mouse 5");

            // Special keys
        case VK_BACK: return xorstr_("Backspace");
        case VK_TAB: return xorstr_("Tab");
        case VK_RETURN: return xorstr_("Enter");
        case VK_PAUSE: return xorstr_("Pause");
        case VK_CAPITAL: return xorstr_("Caps Lock");
        case VK_ESCAPE: return xorstr_("Escape");
        case VK_SPACE: return xorstr_("Space");
        case VK_PRIOR: return xorstr_("Page Up");
        case VK_NEXT: return xorstr_("Page Down");
        case VK_END: return xorstr_("End");
        case VK_HOME: return xorstr_("Home");
        case VK_LEFT: return xorstr_("Left");
        case VK_UP: return xorstr_("Up");
        case VK_RIGHT: return xorstr_("Right");
        case VK_DOWN: return xorstr_("Down");
        case VK_SNAPSHOT: return xorstr_("Print Screen");
        case VK_INSERT: return xorstr_("Insert");
        case VK_DELETE: return xorstr_("Delete");

            // Windows keys
        case VK_LWIN: return xorstr_("Left Win");
        case VK_RWIN: return xorstr_("Right Win");
        case VK_APPS: return xorstr_("Menu");

            // Numpad specific
        case VK_NUMPAD0: return xorstr_("Num 0");
        case VK_NUMPAD1: return xorstr_("Num 1");
        case VK_NUMPAD2: return xorstr_("Num 2");
        case VK_NUMPAD3: return xorstr_("Num 3");
        case VK_NUMPAD4: return xorstr_("Num 4");
        case VK_NUMPAD5: return xorstr_("Num 5");
        case VK_NUMPAD6: return xorstr_("Num 6");
        case VK_NUMPAD7: return xorstr_("Num 7");
        case VK_NUMPAD8: return xorstr_("Num 8");
        case VK_NUMPAD9: return xorstr_("Num 9");
        case VK_MULTIPLY: return xorstr_("Num *");
        case VK_ADD: return xorstr_("Num +");
        case VK_SEPARATOR: return xorstr_("Separator");
        case VK_SUBTRACT: return xorstr_("Num -");
        case VK_DECIMAL: return xorstr_("Num .");
        case VK_DIVIDE: return xorstr_("Num /");
        case VK_NUMLOCK: return xorstr_("Num Lock");

            // Function keys
        case VK_F1: return xorstr_("F1");
        case VK_F2: return xorstr_("F2");
        case VK_F3: return xorstr_("F3");
        case VK_F4: return xorstr_("F4");
        case VK_F5: return xorstr_("F5");
        case VK_F6: return xorstr_("F6");
        case VK_F7: return xorstr_("F7");
        case VK_F8: return xorstr_("F8");
        case VK_F9: return xorstr_("F9");
        case VK_F10: return xorstr_("F10");
        case VK_F11: return xorstr_("F11");
        case VK_F12: return xorstr_("F12");

            // Modifier
        case VK_LSHIFT: return xorstr_("Left Shift");
        case VK_RSHIFT: return xorstr_("Right Shift");
        case VK_LCONTROL: return xorstr_("Left Ctrl");
        case VK_RCONTROL: return xorstr_("Right Ctrl");
        case VK_LMENU: return xorstr_("Left Alt");
        case VK_RMENU: return xorstr_("Right Alt");

            // Media keys
        case VK_VOLUME_MUTE: return xorstr_("Vol Mute");
        case VK_VOLUME_DOWN: return xorstr_("Vol Down");
        case VK_VOLUME_UP: return xorstr_("Vol Up");
        case VK_MEDIA_NEXT_TRACK: return xorstr_("Next Track");
        case VK_MEDIA_PREV_TRACK: return xorstr_("Prev Track");
        case VK_MEDIA_STOP: return xorstr_("Media Stop");
        case VK_MEDIA_PLAY_PAUSE: return xorstr_("Play/Pause");

        default:
            // For standard keys (letters, numbers, etc.)
            UINT scanCode = MapVirtualKey(vKey, MAPVK_VK_TO_VSC);
            if (GetKeyNameTextA(scanCode << 16, keyName, sizeof(keyName))) {
                return keyName;
            }

            // If we still don't have a name, return the hex value
            snprintf(keyName, sizeof(keyName), xorstr_("Key 0x%X"), vKey);
            return keyName;
        }
    }

    // Check if any key is pressed and return its virtual key code
    int GetPressedKey() {
        // Check mouse buttons first
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) return VK_LBUTTON;
        if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) return VK_RBUTTON;
        if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) return VK_MBUTTON;
        if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) return VK_XBUTTON1;
        if (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) return VK_XBUTTON2;

        // Check all possible virtual key codes
        for (int i = 0; i < 256; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                return i;
            }
        }
        return 0;
    }

public:
    void Update() {
        for (auto& hotkey : hotkeys) {
            bool currentlyPressed = IsKeyDown(hotkey);
            bool wasPressed = hotkey.isPressed.exchange(currentlyPressed);

            switch (hotkey.mode.load()) {
            case HotkeyMode::Hold:
                hotkey.state.store(currentlyPressed);
                break;
            case HotkeyMode::Toggle:
                if (currentlyPressed && !wasPressed) {
                    hotkey.state.store(!hotkey.state.load());
                }
                break;
            case HotkeyMode::HoldOff:
                hotkey.state.store(!currentlyPressed);
                break;
            case HotkeyMode::Always:
                hotkey.state.store(true);
                break;
            default:
                hotkey.state.store(false);
                break;
            }
        }
    }

    bool IsActive(HotkeyIndex index) {
        return hotkeys[static_cast<size_t>(index)].state.load();
    }

    bool RenderHotkey(const char* label, HotkeyIndex index) {
        bool changed = false;
        Hotkey& hotkey = hotkeys[static_cast<size_t>(index)];
    
        ImGui::PushID(static_cast<int>(index));

        float buttonHeight = ImGui::GetFrameHeight();
        float textHeight = ImGui::GetTextLineHeight();
        float verticalOffset = (buttonHeight - textHeight) * 0.5f;

        ImGui::BeginGroup();
    
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + verticalOffset);

        if (hotkey.state.load()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        }

        ImGui::Text(xorstr_("%s"), label);

        if (hotkey.state.load()) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - verticalOffset);

        if (ImGui::Button(GetKeyName(hotkey), ImVec2(100, buttonHeight))) {
            ImGui::OpenPopup(xorstr_("Change Hotkey"));
        }

        ImGui::EndGroup();

        if (ImGui::BeginPopupContextItem(xorstr_("ModeMenu"))) {
            const char* modeNames[] = { xorstr_("Off"), xorstr_("Hold"), xorstr_("Toggle"), xorstr_("Hold Off"), xorstr_("Always")};
            HotkeyMode currentMode = hotkey.mode.load();
            for (int i = 0; i < IM_ARRAYSIZE(modeNames); i++) {
                if (ImGui::Selectable(modeNames[i], static_cast<int>(currentMode) == i)) {
                    hotkey.mode.store(static_cast<HotkeyMode>(i));
                    changed = true;
                }
            }
            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal(xorstr_("Change Hotkey"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text(xorstr_("Press any key or mouse button (Esc to clear)..."));
        
            int pressedKey = GetPressedKey();
            if (pressedKey == VK_ESCAPE) {
                if (hotkey.type != InputType::None || hotkey.vKey != 0) {
                    hotkey.type = InputType::None;
                    hotkey.vKey = 0;
                    changed = true;
                }
                ImGui::CloseCurrentPopup();
            }
            else if (pressedKey != 0) {
                if (hotkey.type != InputType::VirtualKey || hotkey.vKey != pressedKey) {
                    hotkey.type = InputType::VirtualKey;
                    hotkey.vKey = pressedKey;
                    changed = true;
                }
                ImGui::CloseCurrentPopup();
            }
        
            ImGui::EndPopup();
        }

        ImGui::PopID();
        return changed;
    }
};