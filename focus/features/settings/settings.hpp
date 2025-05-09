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

#include "../imgui/imgui.h"
#include "hotkeys.hpp"

struct weaponData {
    std::string weaponname = "";
    bool rapidfire = false;
    std::vector<int> attachments;
    std::vector<std::vector<float>> values;

    int triggerBurstDuration = 0;
	int triggerFireDelay = 0;

    bool operator==(const weaponData& other) const {
        return weaponname == other.weaponname &&
            rapidfire == other.rapidfire &&
			attachments == other.attachments &&
            values == other.values &&
            triggerBurstDuration == other.triggerBurstDuration &&
            triggerFireDelay == other.triggerFireDelay;
    }
};

struct characterData {
    std::string charactername = "";
    std::vector<weaponData> weapondata;
    std::vector<bool> options = { false, false };
    std::vector<int> selectedweapon;
};

struct pidSettings {
    int pidPreset = 0;
    float proportional = 0.0f;
	float integral = 0.0f;
	float derivative = 0.0f;
    float rampUpTime = 0.0f;
};

struct colourAimbotSettings {
    int detectionPreset = 0;
    int maxTrackAge = 0;
    int trackSmoothingFactor = 0;
    int trackConfidenceRate = 0;
    int maxClusterDistance = 0;
    int maxClusterDensityDifferential = 0;
    int minDensity = 0;
    int minArea = 0;
    int aimHeight = 0;
    bool debugView = false;
};

struct aiAimbotSettings {
    int provider = 0;
    int hitbox = 0;
    int confidence = 0;
    bool forceHitbox = false;
};

struct triggerBotSettings {
    int detectionMethod = 0;
    float sensitivity = 0.f;
    int radius = 0;
    bool showDebug = false;
    int sleepTime = 0;
};

struct aimbotData {
	int correctionX = 0;
	int correctionY = 0;
	bool enabled = false;
    int type = 0;
    int maxDistance = 0;
    int aimFov = 0;
    bool limitDetectorFps = false;
    int verticalCorrection = 0;
    
    pidSettings pidSettings;
    colourAimbotSettings colourAimbotSettings;
	aiAimbotSettings aiAimbotSettings;
    triggerBotSettings triggerSettings;
};

struct globalSettings {
    bool potato = true;
    int sensitivityCalculator = 0;
    std::vector<bool> characterDetectors = { false };
    std::vector<bool> weaponDetectors = { false, false, false };

    std::vector<float> sensitivity = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, };
    int aspect_ratio = 0;
    float fov = 0;
};

struct overlaySettings {
    bool showInfo = false;

    float magnifierZoom = 0.f;
    int magnifierSize = 0;
};

struct miscSettings {
    int aimKeyMode = 0;
    int recoilKeyMode = 0;
    int quickPeekDelay = 0;
    HotkeySystem hotkeys;
    overlaySettings overlay;
};

struct activeState {
	int selectedCharacterIndex = 0;
	bool isPrimaryActive = true;
	bool weaponOffOverride = false;
	bool weaponDataChanged = false;
	bool pidDataChanged = false;
    int peekDirection = 0;

	std::vector<float> sensMultiplier = { 1.0f, 1.0f };
    std::vector<float> sensMultiplier_SensOnly = { 1.0f, 1.0f };
    float fovSensitivityModifier = 1.f;
};

class Settings 
{
public:
    // Global settings
    globalSettings globalSettings;

	// Aimbot data
    aimbotData aimbotData;

    // Character data
    std::vector<characterData> characters;

	// Active state
	activeState activeState;

    // Misc Settings
	miscSettings misc;

    void readSettings(const std::string& filename, bool clearExisting, bool updateAimbotInfo);
    void saveSettings(const std::string& filename);
};

struct Globals
{
    std::atomic<bool> shutdown{ false };
    std::atomic<bool> initshutdown{ false };
    std::atomic<bool> done{ false };

    struct FileSystem {
        std::vector<std::string> configFiles;
        std::string activeFile = "";
        std::atomic<bool> unsavedChanges{ false };
        std::atomic<int> activeFileIndex{ 0 };
    } filesystem;

    struct Startup {
        std::atomic<bool> passedstartup{ false };
        std::atomic<bool> mouse_driver{ false };
        std::atomic<bool> keyboard_driver{ false };
        std::atomic<bool> files{ false };
        std::atomic<bool> dxgi{ false };
        std::atomic<bool> marker{ false };
        std::atomic<bool> hasFinished{ false };
        std::atomic<int> avx{ -1 };
    } startup;

    struct MouseInfo {
        std::atomic<bool> l_mouse_down{ false };
        std::atomic<bool> r_mouse_down{ false };
        std::atomic<ULONG> marker{ false };
    } mouseinfo;

    struct Capture {
        cv::Mat desktopMat;
        std::mutex desktopMutex_;
        std::atomic<int> desktopWidth{ 0 };
        std::atomic<int> desktopHeight{ 0 };
        std::atomic<int> desktopCenterX{ 0 };
        std::atomic<int> desktopCenterY{ 0 };
        std::atomic<bool> initDims{ false };
    } capture;

    struct Engine {
        std::atomic<float> inferenceTimeMs{ 0.0f };
    } engine;
};

struct Constants {
    static constexpr int SIEGE360DIST = 7274; //82 FOV
    static constexpr int RUST360DIST = 8732; //90 FOV
    static constexpr int OW360DIST = 6284; //ALL FOV
};

extern Constants constants;
extern Globals globals;
extern Settings settings;