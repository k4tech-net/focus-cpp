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
        bool hasFinished;
    } startup;

    struct weaponInfo {
        std::vector<Settings> weapons;
        Settings selectedWeapon;

        bool currautofire;
        int currxdeadtime = 1;

        std::string weaponsText;
        int selectedItem;
    } weaponinfo;
};