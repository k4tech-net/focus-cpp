#include "overlay.hpp"
#include <dwmapi.h>
#include <gdiplus.h>
#include <string>
#include <sstream>
#include <iostream>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

Overlay::Overlay() {
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

Overlay::~Overlay() {
    Shutdown();

    // Shutdown GDI+
    if (gdiplusToken != 0) {
        GdiplusShutdown(gdiplusToken);
        gdiplusToken = 0;
    }
}

void Overlay::Initialize() {
    if (hwnd != nullptr || threadRunning) return; // Already initialized

    // Reset thread state
    shouldExit = false;
    forceRedraw = true; // Force redraw when initialized

    // If there's an existing thread, ensure it's properly joined
    if (renderThread.joinable()) {
        renderThread.join();
    }

    // Start a new thread
    threadRunning = true;
    renderThread = std::thread(&Overlay::RenderThreadProc, this);
}

void Overlay::Shutdown() {
    // Signal the thread to exit
    shouldExit = true;

    // Wait for thread to finish
    if (renderThread.joinable()) {
        renderThread.join();
    }

    threadRunning = false;
}

void Overlay::UpdateState() {
    // Get current settings
    bool enabled = settings.misc.overlay.enabled;
    bool showInfo = settings.misc.overlay.showInfo;
    bool magnifierActive = settings.misc.hotkeys.IsActive(HotkeyIndex::MagnifierKey);

    // Check for changes
    bool stateChanged = (enabled != lastEnabled || showInfo != lastShowInfo || magnifierActive != lastMagnifierActive);

    // Update tracking variables
    lastEnabled = enabled;
    lastShowInfo = showInfo;
    lastMagnifierActive = magnifierActive;

    // If enabled state changed, initialize or shutdown
    if (enabled && !threadRunning) {
        Initialize();
    }
    else if (!enabled && threadRunning) {
        Shutdown();
    }
    else if (stateChanged && threadRunning) {
        // If other settings changed but thread is running, force a redraw
        forceRedraw = true;
    }
}

void Overlay::RenderThreadProc() {
    // Register window class
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"FocusOverlayClass";

    if (!RegisterClassEx(&wc)) {
        std::cerr << "Failed to register overlay window class" << std::endl;
        threadRunning = false;
        return;
    }

    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create window
    hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        L"FocusOverlayClass",
        L"Focus Overlay",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, GetModuleHandle(NULL), this);

    if (!hwnd) {
        std::cerr << "Failed to create overlay window" << std::endl;
        UnregisterClass(L"FocusOverlayClass", GetModuleHandle(NULL));
        threadRunning = false;
        return;
    }

    // Make window transparent
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);

    // Exclude from capture
    SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);

    // Show the window
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(hwnd);

    // Create DC and memory bitmap once
    HDC hdc = GetDC(hwnd);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, screenWidth, screenHeight);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Create clear rect brush once
    HBRUSH blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

    // Setup fonts and other GDI objects
    HFONT font = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT oldFont = (HFONT)SelectObject(memDC, font);

    // Initial clear of the bitmap
    RECT rc = { 0, 0, screenWidth, screenHeight };
    FillRect(memDC, &rc, blackBrush);

    // Message loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    // Last drawn positions for dirty rect tracking
    RECT lastInfoRect = { 0 };
    RECT lastMagnifierRect = { 0 };
    bool wasMagnifierActive = false;

    // Frame timing
    const int TARGET_FPS = 60; // Lower target FPS to save resources
    const auto FRAME_DURATION = std::chrono::milliseconds(1000 / TARGET_FPS);
    auto lastRedrawTime = std::chrono::steady_clock::now();

    while (!shouldExit && msg.message != WM_QUIT) {
        // Process messages
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Check if we need to exit or if settings have changed
        bool needForceRedraw = false;
        bool showInfoEnabled = false;
        bool magnifierActive = false;

        // Get current settings
        if (!settings.misc.overlay.enabled) {
            break;
        }

        showInfoEnabled = settings.misc.overlay.showInfo;
        magnifierActive = settings.misc.hotkeys.IsActive(HotkeyIndex::MagnifierKey);

        if (forceRedraw) {
            needForceRedraw = true;
            forceRedraw = false;
        }

        // Detect magnifier state change
        bool magnifierStateChanged = (wasMagnifierActive != magnifierActive);

        // Limit frame rate
        auto currentTime = std::chrono::steady_clock::now();
        auto timeSinceLastFrame = currentTime - lastRedrawTime;
        bool timeForRedraw = timeSinceLastFrame >= FRAME_DURATION;

        // Need to check for changes in game state that affect overlay info
        int currentSelectedCharacterIndex = settings.activeState.selectedCharacterIndex;
        bool currentWeaponOffOverride = settings.activeState.weaponOffOverride;
        bool currentIsPrimaryActive = settings.activeState.isPrimaryActive;
        bool currentAimbotEnabled = settings.aimbotData.enabled;

        // Check if any game state changed
        bool infoChanged = (currentSelectedCharacterIndex != lastSelectedCharacterIndex) ||
            (currentWeaponOffOverride != lastWeaponOffOverride) ||
            (currentIsPrimaryActive != lastIsPrimaryActive) ||
            (currentAimbotEnabled != lastAimbotEnabled);

        bool needRedraw = needForceRedraw;

        // Only need to redraw if info panel is enabled and something changed
        if (showInfoEnabled && (infoChanged || needForceRedraw || lastShowInfo != showInfoEnabled)) {
            needRedraw = true;
        }

        // Handle magnifier redraw
        if (magnifierActive && (timeForRedraw || needForceRedraw || magnifierStateChanged)) {
            needRedraw = true;
        }

        // IMPORTANT: Handle magnifier deactivation specially
        bool needClearMagnifier = wasMagnifierActive && !magnifierActive;

        // Update tracking variables
        lastShowInfo = showInfoEnabled;
        lastSelectedCharacterIndex = currentSelectedCharacterIndex;
        lastWeaponOffOverride = currentWeaponOffOverride;
        lastIsPrimaryActive = currentIsPrimaryActive;
        lastAimbotEnabled = currentAimbotEnabled;

        // Ensure window stays on top (do this less frequently)
        static int topMostCounter = 0;
        if (++topMostCounter >= 30) {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            topMostCounter = 0;
        }

        // Special handling for clearing the magnifier
        if (needClearMagnifier && lastMagnifierRect.right > lastMagnifierRect.left) {
            // Clear the magnifier area and update screen with black
            RECT clearRect = DrawMagnifier(memDC, true);

            // Update screen with cleared area
            BitBlt(hdc, clearRect.left, clearRect.top,
                clearRect.right - clearRect.left,
                clearRect.bottom - clearRect.top,
                memDC, clearRect.left, clearRect.top, SRCCOPY);

            // Reset magnifier rect
            lastMagnifierRect = { 0 };

            // Force another clear on next frame to ensure it's completely gone
            forceRedraw = true;
        }

        // Update magnifier active state for next iteration
        wasMagnifierActive = magnifierActive;

        if (!needRedraw && !needClearMagnifier) {
            // Sleep less when magnifier is active to be more responsive
            std::this_thread::sleep_for(magnifierActive ? std::chrono::milliseconds(0) : std::chrono::milliseconds(5));
            continue;
        }

        if (timeForRedraw) {
            lastRedrawTime = currentTime;
        }

        // Skip the rest of the drawing if we only needed to clear the magnifier
        if (needClearMagnifier && !needRedraw) {
            continue;
        }

        // Normal rendering path
        // Clear for redraw
        if (needForceRedraw) {
            // Full clear
            FillRect(memDC, &rc, blackBrush);
            lastInfoRect = { 0 };
            lastMagnifierRect = { 0 };
        }
        else {
            // Clear only the necessary areas
            if (lastShowInfo && !showInfoEnabled && lastInfoRect.right > lastInfoRect.left) {
                FillRect(memDC, &lastInfoRect, blackBrush);
                lastInfoRect = { 0 };
            }

            // Clear areas for redraw
            if (showInfoEnabled && lastInfoRect.right > lastInfoRect.left) {
                FillRect(memDC, &lastInfoRect, blackBrush);
            }

            if (magnifierActive && lastMagnifierRect.right > lastMagnifierRect.left) {
                FillRect(memDC, &lastMagnifierRect, blackBrush);
            }
        }

        // Draw info panel if enabled
        if (showInfoEnabled) {
            lastInfoRect = DrawInfo(memDC);

            // Update the info area
            if (lastInfoRect.right > lastInfoRect.left && lastInfoRect.bottom > lastInfoRect.top) {
                BitBlt(hdc, lastInfoRect.left, lastInfoRect.top,
                    lastInfoRect.right - lastInfoRect.left,
                    lastInfoRect.bottom - lastInfoRect.top,
                    memDC, lastInfoRect.left, lastInfoRect.top, SRCCOPY);
            }
        }

        // Draw magnifier if active
        if (magnifierActive) {
            lastMagnifierRect = DrawMagnifier(memDC);

            // Update the magnifier area
            if (lastMagnifierRect.right > lastMagnifierRect.left && lastMagnifierRect.bottom > lastMagnifierRect.top) {
                BitBlt(hdc, lastMagnifierRect.left, lastMagnifierRect.top,
                    lastMagnifierRect.right - lastMagnifierRect.left,
                    lastMagnifierRect.bottom - lastMagnifierRect.top,
                    memDC, lastMagnifierRect.left, lastMagnifierRect.top, SRCCOPY);
            }
        }
    }

    // Cleanup GDI resources
    SelectObject(memDC, oldFont);
    DeleteObject(font);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
    ReleaseDC(hwnd, hdc);

    // Cleanup window
    DestroyWindow(hwnd);
    UnregisterClass(L"FocusOverlayClass", GetModuleHandle(NULL));
    hwnd = nullptr;
    threadRunning = false;
}

RECT Overlay::DrawInfo(HDC hdc) {
    int y_pos = 10;
    RECT boundingRect = { 10, y_pos, 300, y_pos };

    // Get reference to settings
    bool aimbotEnabled = settings.aimbotData.enabled;
    bool weaponOffOverride = settings.activeState.weaponOffOverride;
    bool isPrimaryActive = settings.activeState.isPrimaryActive;
    int selectedCharacterIndex = settings.activeState.selectedCharacterIndex;

    // Draw aimbot status
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(255, 255, 255));

    RECT textRect = { 10, y_pos, 300, y_pos + 20 };
    std::string text = std::string(xorstr_("Aimbot: ")) + (aimbotEnabled ? xorstr_("ON") : xorstr_("OFF"));
    DrawTextA(hdc, text.c_str(), (int)text.length(), &textRect, DT_LEFT);
    boundingRect.bottom = textRect.bottom;

    y_pos += 20;

    // Draw weapon status
    RECT weaponRect = { 10, y_pos, 300, y_pos + 20 };
    if (weaponOffOverride) {
        SetTextColor(hdc, RGB(255, 0, 0));
        DrawTextA(hdc, xorstr_("Weapon: DISABLED"), -1, &weaponRect, DT_LEFT);
    }
    else if (isPrimaryActive) {
        SetTextColor(hdc, RGB(0, 255, 0));
        DrawTextA(hdc, xorstr_("Weapon: PRIMARY"), -1, &weaponRect, DT_LEFT);
    }
    else {
        SetTextColor(hdc, RGB(0, 255, 0));
        DrawTextA(hdc, xorstr_("Weapon: SECONDARY"), -1, &weaponRect, DT_LEFT);
    }
    boundingRect.bottom = weaponRect.bottom;

    y_pos += 20;

    // Draw selected character
    if (selectedCharacterIndex < settings.characters.size() && !settings.characters.empty()) {
        RECT charRect = { 10, y_pos, 300, y_pos + 20 };
        std::string charText = std::string(xorstr_("Character: ")) +
            settings.characters[selectedCharacterIndex].charactername;
        SetTextColor(hdc, RGB(255, 255, 255));
        DrawTextA(hdc, charText.c_str(), (int)charText.length(), &charRect, DT_LEFT);
        boundingRect.bottom = charRect.bottom;
        y_pos += 20;
    }

    // Optionally show inference time for AI aimbot
    if (settings.aimbotData.type == 1 && aimbotEnabled) {
        RECT msRect = { 10, y_pos, 300, y_pos + 20 };
        std::stringstream ms_buffer;
        float ms = globals.inferenceTimeMs.load();
        ms_buffer << xorstr_("Inference: ") << ms << " ms";

        // Color-code based on performance
        COLORREF timeColor;
        if (ms <= 10.0f)
            timeColor = RGB(0, 255, 0);  // Green for fast
        else if (ms <= 30.0f)
            timeColor = RGB(255, 255, 0);  // Yellow for medium
        else if (ms <= 50.0f)
            timeColor = RGB(255, 165, 0);  // Orange for slow
        else
            timeColor = RGB(255, 0, 0);  // Red for very slow

        SetTextColor(hdc, timeColor);
        DrawTextA(hdc, ms_buffer.str().c_str(), -1, &msRect, DT_LEFT);
        boundingRect.bottom = msRect.bottom;
    }

    // Add some padding to the bounding rect
    boundingRect.right += 10;
    boundingRect.bottom += 5;

    return boundingRect;
}

RECT Overlay::DrawMagnifier(HDC hdc, bool shouldClear) {
    // Get magnifier settings
    float zoom = settings.misc.overlay.magnifierZoom;
    int size = settings.misc.overlay.magnifierSize;

    // Get screen center
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int centerX = screenWidth / 2;
    int centerY = screenHeight / 2;

    // Calculate source and destination rectangles
    int sourceSize = (int)(size / zoom);
    int sourceX = centerX - sourceSize / 2;
    int sourceY = centerY - sourceSize / 2;
    int destX = centerX - size / 2;
    int destY = centerY - size / 2;

    // Create a RECT for the magnifier area
    RECT magnifierRect = {
        destX,
        destY,
        destX + size,
        destY + size
    };

    // If we're just clearing the area, fill with black and return
    if (shouldClear) {
        // Use GDI+ to ensure proper clearing
        Graphics graphics(hdc);
        SolidBrush blackBrush(Color(255, 0, 0, 0));
        graphics.FillRectangle(&blackBrush, destX, destY, size, size);
        return magnifierRect;
    }

    // Get current desktop image safely
    cv::Mat desktopMat;
    {
        std::lock_guard<std::mutex> lock(globals.capture.desktopMutex_);
        if (!globals.capture.desktopMat.empty()) {
            // Make sure source rectangle is within bounds
            sourceX = std::max(0, std::min(sourceX, globals.capture.desktopMat.cols - sourceSize));
            sourceY = std::max(0, std::min(sourceY, globals.capture.desktopMat.rows - sourceSize));
            sourceSize = std::min(sourceSize, std::min(globals.capture.desktopMat.cols - sourceX, globals.capture.desktopMat.rows - sourceY));

            if (sourceSize > 0) {
                cv::Rect roi(sourceX, sourceY, sourceSize, sourceSize);
                desktopMat = globals.capture.desktopMat(roi).clone();
            }
        }
    }

    // Use GDI+ to draw the magnifier
    Graphics graphics(hdc);

    // If we got valid desktop image
    if (!desktopMat.empty()) {
        // Resize the image to destination size using GDI+
        cv::Mat resizedMat;
        cv::resize(desktopMat, resizedMat, cv::Size(size, size), 0, 0, cv::INTER_LINEAR);

        // Create GDI+ Bitmap from OpenCV Mat
        Bitmap* gdiBitmap = nullptr;

        // Convert Mat to proper format for GDI+
        if (resizedMat.channels() == 4) {
            // BGRA format
            gdiBitmap = new Bitmap(resizedMat.cols, resizedMat.rows, resizedMat.step,
                PixelFormat32bppARGB, resizedMat.data);
        }
        else if (resizedMat.channels() == 3) {
            // Convert BGR to BGRA for GDI+
            cv::Mat tempMat;
            cv::cvtColor(resizedMat, tempMat, cv::COLOR_BGR2BGRA);
            gdiBitmap = new Bitmap(tempMat.cols, tempMat.rows, tempMat.step,
                PixelFormat32bppARGB, tempMat.data);
        }

        if (gdiBitmap) {
            // Draw the bitmap
            graphics.DrawImage(gdiBitmap, destX, destY, size, size);

            // Draw border around magnifier
            Pen whitePen(Color(255, 255, 255, 255), 2);
            graphics.DrawRectangle(&whitePen, destX, destY, size, size);

            // Clean up
            delete gdiBitmap;
        }
    }
    else {
        // If no desktop image, just draw a border to show where magnifier would be
        Pen redPen(Color(255, 255, 0, 0), 2);
        SolidBrush redBrush(Color(255, 255, 0, 0));

        // Draw rectangle border
        graphics.DrawRectangle(&redPen, destX, destY, size, size);

        // Draw text indicating no image
        FontFamily fontFamily(L"Arial");
        Font font(&fontFamily, 12, FontStyleRegular, UnitPoint);
        PointF textPosition(static_cast<REAL>(destX + 10), static_cast<REAL>(destY + size / 2 - 10));

        graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        graphics.DrawString(L"No screen capture available", -1, &font, textPosition, &redBrush);
    }

    return magnifierRect;
}

LRESULT CALLBACK Overlay::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Get the overlay instance
    Overlay* overlay = nullptr;

    if (msg == WM_NCCREATE) {
        // Store overlay instance pointer during window creation
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        overlay = (Overlay*)cs->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)overlay);
    }
    else {
        // Retrieve the overlay instance pointer
        overlay = (Overlay*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}