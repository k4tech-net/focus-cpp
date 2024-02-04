#pragma once

#include "settings.hpp"

struct Globals
{
    int selectedItem;
    Settings selectedWeapon;

    std::vector<Settings> weapons;
};