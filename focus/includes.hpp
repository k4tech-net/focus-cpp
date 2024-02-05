#pragma once

#include <Windows.h>
#include <sstream>
#include <chrono>
#include <thread>
#include <math.h>
#include <stdio.h>

#include "mouse_driver/mouse.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "features/settings/globals.hpp"

extern Globals g;