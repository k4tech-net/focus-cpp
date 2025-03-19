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
    std::vector<bool> options;
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
    int burstDuration = 0;
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
    bool potato = false;
    int sensitivityCalculator = 0;
    std::vector<bool> characterDetectors = { false };
    std::vector<bool> weaponDetectors = { false, false };

    std::vector<float> sensitivity = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, };
    int aspect_ratio = 0;
    float fov = 0;
};

struct overlaySettings {
    bool enabled = false;
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

    std::vector<std::string> convertAllLegacyConfigs();
    bool isLegacyConfig(const std::string& filename);
    void convertLegacyConfig(const std::string& filename);
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

    std::atomic<float> inferenceTimeMs{ 0.0f };
};

struct Constants {
    static constexpr int SIEGE360DIST = 7274; //82 FOV
    static constexpr int RUST360DIST = 8732; //90 FOV
    static constexpr int OW360DIST = 6284; //ALL FOV
};

extern Constants constants;
extern Globals globals;
extern Settings settings;