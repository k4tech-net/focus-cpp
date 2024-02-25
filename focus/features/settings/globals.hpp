#pragma once

#include "settings.hpp"

struct Globals
{
    bool shutdown;
    bool initshutdown;

    GLFWwindow* window;

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

		std::vector<Settings> characters;
		Settings selectedCharacter;

		int selectedCharacterIndex = 0;
        int selectedPrimary = 0;
		int selectedSecondary = 0;

        bool primaryAutofire;
		bool secondaryAutofire;
		std::vector<bool> characterOptions;

		bool currAutofire;
        
		bool isPrimaryActive = true;
		weaponData activeWeapon; // Put this in the keybind controller
        bool weaponOffOverride;

        std::string jsonData;
	} characterinfo;

    //struct weaponInfo {
    //    std::vector<Settings> weapons;
    //    Settings selectedWeapon;

    //    bool currautofire;
    //    int currxdeadtime = 1;

    //    std::string weaponsText;
    //    int selectedItem;
    //} weaponinfo;
};