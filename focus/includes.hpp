#pragma once

#include <Windows.h>
#include <math.h>
#include <stdio.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "plugins/TextEditor.h"

#include "features/utils/utils.hpp"

#define CHI g.characterinfo

extern Globals g;
extern DXGI dx;
extern Settings cfg;
extern TextEditor editor;
extern Utils ut;
extern Mouse ms;