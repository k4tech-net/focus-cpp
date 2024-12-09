#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>
#include <xorstr.hpp>
#include <variant>
#include <mutex>
#include <random>
#include <Windows.h>

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include <nlohmann/json.hpp>

#include "../imgui/imgui.h"
#include "hotkeys.hpp"

using json = nlohmann::json;

struct weaponData {
    std::string weaponname = "";
    bool rapidfire = false;
    std::vector<int> attachments = { 0, 0, 0 };
    std::vector<std::vector<float>> values;

    bool operator==(const weaponData& other) const {
        return weaponname == other.weaponname &&
            rapidfire == other.rapidfire &&
			attachments == other.attachments &&
            values == other.values;
    }
};

struct characterData {
    std::string charactername = "";
    std::vector<weaponData> weapondata;
    std::vector<bool> options = { false, false, false, false, false };
    std::vector<int> selectedweapon = { 0, 0 };
};

struct pidSettings {
    int pidPreset = 0;
    float proportional = 0.0f;
	float integral = 0.0f;
	float derivative = 0.0f;
    float rampUpTime = 0.0f;
};

struct aimbotData {
	int correctionX = 0;
	int correctionY = 0;
	bool enabled = false;
    int type = 0;
    int provider = 0;
    int maxDistance = 0;
    int hitbox = 0;
    int confidence = 10;
    bool forceHitbox = false;
    int fov = 0;
    pidSettings pidSettings;
};

struct extraSettings {
    int aimKeyMode = 0;
    int recoilKeyMode = 0;
};

class Settings 
{
public:
    // Global settings
    std::string mode = "";
    bool potato = false;
    aimbotData aimbotData;

    // Character mode settings
    std::vector<std::string> wpn_keybinds;
    std::vector<std::string> aux_keybinds;

    HotkeySystem hotkeys;

    // Game mode settings
    std::string game = "";
    std::vector<float> sensitivity;
    std::string crouch_keybind;
    int aspect_ratio = 0;
    float fov = 0;
    float fovSensitivityModifier = 1.f;
    int quickPeekDelay = 0;

    // Character data
    std::vector<characterData> characters;

    // Active state
    int selectedCharacterIndex = 0;
    bool isPrimaryActive = true;
    bool weaponOffOverride = false;
    bool weaponDataChanged = false;
    bool pidDataChanged = false;
    std::vector<float> sensMultiplier = { 1.0f, 1.0f };

    // Misc Settings
	extraSettings extras;

    void readSettings(const std::string& filename, bool clearExisting, bool updateAimbotInfo);
    void saveSettings(const std::string& filename);
};

struct Globals
{
    bool shutdown = false;
    bool initshutdown = false;
    bool done = false;

    struct FileSystem {
        std::vector<std::string> configFiles;
        std::string activeFile = "";
        bool unsavedChanges = false;
        int activeFileIndex = 0;
    } filesystem;

    struct Startup {
        bool passedstartup = false;
        bool mouse_driver = false;
        bool keyboard_driver = false;
        bool files = false;
        bool dxgi = false;
        bool marker = false;
        bool hasFinished = false;
        int avx = -1;
    } startup;

    struct MouseInfo {
		std::atomic<bool> l_mouse_down = false;
		std::atomic<bool> r_mouse_down = false;
        std::atomic<ULONG> marker = 0;
    } mouseinfo;

    struct Capture {
        cv::Mat desktopMat;
        std::mutex desktopMutex_;
        int desktopWidth = 0;
		int desktopHeight = 0;
        int desktopCenterX = 0;
		int desktopCenterY = 0;
        bool initDims = false;
    } capture;
};

struct Constants {
    static constexpr int SIEGE360DIST = 7274; //82 FOV
    static constexpr int RUST360DIST = 8732; //90 FOV
    static constexpr int OW360DIST = 6284; //ALL FOV
};

extern Constants constants;
extern Globals globals;
extern Settings settings;