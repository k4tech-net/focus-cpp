#pragma once

#include <Windows.h>
#include <math.h>
#include <stdio.h>
#include <xorstr.hpp>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "plugins/TextEditor.h"

#include "features/utils/utils.hpp"

#define CHI g.characterinfo

extern Globals g;
extern DXGI dx;
extern Settings cfg;
extern TextEditor editor;
extern Utils ut;
extern Mouse ms;