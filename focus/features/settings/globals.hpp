#pragma once

#include "settings.hpp"

struct Globals
{
    int selectedItem;
    Settings selectedWeapon;
    bool shutdown;

    std::vector<Settings> weapons;

    std::string weaponsText;

    struct Editor {
        std::vector<std::string> jsonFiles;
		std::string activeFile;
		bool unsavedChanges;
		int activeFileIndex;
    } editor;
};