#pragma once

#include <Windows.h>
#include <chrono>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <xorstr.hpp>

#include "../includes.hpp"

// Forward declarations
class Settings;
extern Settings settings;
extern struct Globals globals;

class Overlay {
public:
    Overlay();
    ~Overlay();

    // Interface methods
    void Initialize();
    void Shutdown();
    void UpdateState(); // Called from menu.cpp, updates state based on settings

private:
    // Window resources
    HWND hwnd = nullptr;
    ULONG_PTR gdiplusToken = 0;

    // Thread control
    std::thread renderThread;
    std::atomic<bool> shouldExit{ false };
    std::atomic<bool> forceRedraw{ false };
    bool threadRunning = false;

    // State tracking for change detection
    bool lastEnabled = false;
    bool lastShowInfo = false;
    bool lastMagnifierActive = false;

    // Game state tracking for change detection
    int lastSelectedCharacterIndex = -1;
    bool lastWeaponOffOverride = false;
    bool lastIsPrimaryActive = false;
    bool lastAimbotEnabled = false;

    // Internal rendering thread
    void RenderThreadProc();

    // Drawing methods
    RECT DrawInfo(HDC hdc);
    RECT DrawMagnifier(HDC hdc, bool shouldClear = false);

    // Window procedure
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};