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

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct weaponData {
    std::string weaponname = "";
    bool autofire = false;
    std::vector<int> attachments = { 0, 0, 0 };
    std::vector<std::vector<float>> values;

    bool operator==(const weaponData& other) const {
        return weaponname == other.weaponname &&
            autofire == other.autofire &&
			attachments == other.attachments &&
            values == other.values;
    }
};

class Settings 
{
public:
    std::string charactername = "";
	std::vector<weaponData> weapondata;
    std::vector<bool> options = { false, false, false, false, false };
    std::vector<int> defaultweapon = { 0, 0 };

    //// Method to check if two Settings objects are equal
    //bool operator==(const Settings& other) const {
    //    return charactername == other.charactername &&
    //        weapondata == other.weapondata &&
    //        options == other.options;
    //}

    void readSettings(const std::string& filename, std::vector<Settings>& settings, bool clearExisting);
};

struct Globals
{
    bool shutdown = false;
    bool initshutdown = false;
   
    cv::Mat desktopMat;
    std::mutex desktopMutex_;

    bool done = false;

    struct Editor {
        std::vector<std::string> jsonFiles;
        std::string activeFile = "";
        bool unsavedChanges = false;
        int activeFileIndex = 0;
    } editor;

    struct Startup {
        bool passedstartup = false;
        bool driver = false;
        bool files = false;
        bool dxgi = false;
        bool hasFinished = false;
    } startup;

    struct CharacterInfo {
        std::string mode = "";
        bool potato = false;
        std::vector<std::string> wpn_keybinds;
        std::vector<std::string> aux_keybinds;
        
        std::string game = "";
        std::vector<float> sensitivity = { 0, 0, 0, 0, 0 };

        std::vector<Settings> characters;
        Settings selectedCharacter;

        int selectedCharacterIndex = 0;
        int selectedPrimary = 0;
        int selectedSecondary = 0;

        bool primaryAutofire = false;
        bool secondaryAutofire = false;
        std::vector<int> primaryAttachments = { 0, 0, 0 };
		std::vector<int> secondaryAttachments = { 0, 0, 0 };
        std::vector<bool> characterOptions = { false, false, false, false, false };

        bool currAutofire = false;

        bool isPrimaryActive = true;
        weaponData activeWeapon; // Put this in the keybind controller
        float activeWeaponSensXModifier = 0;
        float activeWeaponSensYModifier = 0;
        bool weaponOffOverride = false;

        std::string jsonData = "";

        std::mutex mutex_;
    } characterinfo;
};

extern Globals g;

#define CHI g.characterinfo