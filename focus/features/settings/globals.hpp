#pragma once

#include "settings.hpp"

struct Globals
{
    int selectedItem;
    Settings selectedWeapon;
    bool shutdown;

    std::vector<Settings> weapons;

    std::string weaponsText;
};