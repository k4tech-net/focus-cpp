#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <optional>
#include <fstream>
#include <sstream>
#include <xorstr.hpp>
#include <variant>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct weaponData {
    std::string weaponname;
    bool autofire;
    std::vector<int> attachments;
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
    std::string charactername;
	std::vector<weaponData> weapondata;
    std::vector<bool> options;
    std::vector<int> defaultweapon;

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
    bool shutdown;
    bool initshutdown;

    bool done;

    struct Editor {
        std::vector<std::string> jsonFiles;
        std::string activeFile;
        bool unsavedChanges;
        int activeFileIndex;
    } editor;

    struct Startup {
        bool passedstartup;
        bool driver;
        bool files;
        bool dxgi;
        bool hasFinished;
    } startup;

    struct CharacterInfo {
        std::string mode;
        std::vector<std::string> wpn_keybinds;
        std::vector<std::string> aux_keybinds;
        
        std::string game;
        std::vector<float> sensitivity;

        std::vector<Settings> characters;
        Settings selectedCharacter;

        int selectedCharacterIndex = 0;
        int selectedPrimary = 0;
        int selectedSecondary = 0;

        bool primaryAutofire;
        bool secondaryAutofire;
        std::vector<int> primaryAttachments;
		std::vector<int> secondaryAttachments;
        std::vector<bool> characterOptions;

        bool currAutofire;

        bool isPrimaryActive = true;
        weaponData activeWeapon; // Put this in the keybind controller
        float activeWeaponSensXModifier;
        float activeWeaponSensYModifier;
        bool weaponOffOverride;

        std::string jsonData;
    } characterinfo;

    //struct weaponInfo {
    //    std::vector<Settings> weapons;
    //    Settings selectedWeapon;

    //    bool currautofire;

    //    std::string weaponsText;
    //    int selectedItem;
    //} weaponinfo;
};

extern Globals g;

#define CHI g.characterinfo