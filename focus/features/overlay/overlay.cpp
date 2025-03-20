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
    bool enabled = settings.misc.hotkeys.IsActive(HotkeyIndex::OverlayKey);
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

    // State tracking
    bool isInfoPanelVisible = false;
    bool isMagnifierVisible = false;
    RECT infoRect = { 0 };
    RECT magnifierRect = { 0 };

    // Extended state tracking for new indicators
    bool lastQuickPeekActive = false;
    bool lastHashomPeekActive = false;
    bool lastTriggerBotActive = false;
    bool lastUiHidden = false;
    float lastInferenceTime = 0.0f;
    int lastPeekDirection = 0;

    // Frame timing
    const int TARGET_FPS = 60;
    const auto FRAME_DURATION = std::chrono::milliseconds(1000 / TARGET_FPS);
    auto lastFrameTime = std::chrono::steady_clock::now();

    // Keep track of all dirty regions that need to be updated
    std::vector<RECT> dirtyRects;

    while (!shouldExit && msg.message != WM_QUIT) {
        // Process messages
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Check if we need to exit or if settings changed
        if (!settings.misc.hotkeys.IsActive(HotkeyIndex::OverlayKey)) {
            break;
        }

        // Get current settings
        bool showInfoEnabled = settings.misc.overlay.showInfo;
        bool magnifierActive = settings.misc.hotkeys.IsActive(HotkeyIndex::MagnifierKey);

        // Get current state of the new indicators
        bool quickPeekActive = settings.misc.hotkeys.IsActive(HotkeyIndex::AutoQuickPeek);
        bool hashomPeekActive = settings.misc.hotkeys.IsActive(HotkeyIndex::AutoHashomPeek);
        bool triggerBotActive = settings.misc.hotkeys.IsActive(HotkeyIndex::TriggerKey);
        bool uiHidden = settings.misc.hotkeys.IsActive(HotkeyIndex::HideUiKey);
        float inferenceTime = globals.inferenceTimeMs.load();
        bool peekDirection = settings.activeState.peekDirection;

        // Check for forced redraw
        bool needForceRedraw = forceRedraw;
        if (forceRedraw) {
            forceRedraw = false;
        }

        // Clear dirty rects for this frame
        dirtyRects.clear();

        // Update state for game data
        int currentSelectedCharacterIndex = settings.activeState.selectedCharacterIndex;
        bool currentWeaponOffOverride = settings.activeState.weaponOffOverride;
        bool currentIsPrimaryActive = settings.activeState.isPrimaryActive;
        bool currentAimbotEnabled = settings.aimbotData.enabled;

        // Check if any game state changed
        bool infoChanged = (currentSelectedCharacterIndex != lastSelectedCharacterIndex) ||
            (currentWeaponOffOverride != lastWeaponOffOverride) ||
            (currentIsPrimaryActive != lastIsPrimaryActive) ||
            (currentAimbotEnabled != lastAimbotEnabled) ||
            // Check if any of the new indicators changed
            (quickPeekActive != lastQuickPeekActive) ||
            (hashomPeekActive != lastHashomPeekActive) ||
            (triggerBotActive != lastTriggerBotActive) ||
            (uiHidden != lastUiHidden) ||
            (peekDirection != lastPeekDirection) ||
            // Check if inference time changed significantly (more than 0.5ms)
            (std::abs(inferenceTime - lastInferenceTime) > 0.5f);

        // Check if info visibility changed
        bool infoVisibilityChanged = isInfoPanelVisible != showInfoEnabled;

        // Update all state tracking variables
        lastSelectedCharacterIndex = currentSelectedCharacterIndex;
        lastWeaponOffOverride = currentWeaponOffOverride;
        lastIsPrimaryActive = currentIsPrimaryActive;
        lastAimbotEnabled = currentAimbotEnabled;
        lastQuickPeekActive = quickPeekActive;
        lastHashomPeekActive = hashomPeekActive;
        lastTriggerBotActive = triggerBotActive;
        lastUiHidden = uiHidden;
        lastInferenceTime = inferenceTime;
        peekDirection = lastPeekDirection;

        // Check if time for a frame update or if change occurred
        auto currentTime = std::chrono::steady_clock::now();
        bool timeForFrame = currentTime - lastFrameTime >= FRAME_DURATION;

        // Handle info panel visibility changes
        if (infoVisibilityChanged) {
            if (isInfoPanelVisible && !showInfoEnabled) {
                // Info panel was visible and needs to be hidden - clear the area
                if (infoRect.right > infoRect.left) {
                    FillRect(memDC, &infoRect, blackBrush);
                    dirtyRects.push_back(infoRect);
                }
            }
            isInfoPanelVisible = showInfoEnabled;
        }

        // Determine if we need to update the info panel
        bool needInfoUpdate = isInfoPanelVisible && (infoChanged || needForceRedraw || infoVisibilityChanged);

        // For inference time updates, force more frequent updates if enabled
        if (isInfoPanelVisible && currentAimbotEnabled && settings.aimbotData.type == 1 && timeForFrame) {
            needInfoUpdate = true; // Update regularly when inference is showing
        }

        // Handle magnifier visibility changes
        bool magnifierVisibilityChanged = isMagnifierVisible != magnifierActive;

        // If magnifier was visible but now should be hidden
        if (magnifierVisibilityChanged && isMagnifierVisible && !magnifierActive) {
            // Clear the magnifier area
            if (magnifierRect.right > magnifierRect.left) {
                FillRect(memDC, &magnifierRect, blackBrush);
                dirtyRects.push_back(magnifierRect);
                magnifierRect = { 0 };
            }
        }

        isMagnifierVisible = magnifierActive;

        // Determine if we need to update the magnifier
        bool needMagnifierUpdate = isMagnifierVisible && (timeForFrame || needForceRedraw || magnifierVisibilityChanged);

        // Update topmost status periodically
        /*static int topMostCounter = 0;
        if (++topMostCounter >= 30) {
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            topMostCounter = 0;
        }*/

        // Skip drawing if nothing changed
        if (!needInfoUpdate && !needMagnifierUpdate && dirtyRects.empty()) {
            // Sleep less when magnifier is active to be more responsive
            std::this_thread::sleep_for(magnifierActive ? std::chrono::microseconds(500) : std::chrono::milliseconds(5));
            continue;
        }

        // If we're going to render in this frame, update the time
        if (timeForFrame) {
            lastFrameTime = currentTime;
        }

        // Handle full redraw if needed
        if (needForceRedraw) {
            // Full clear
            FillRect(memDC, &rc, blackBrush);

            // Mark entire screen as dirty
            dirtyRects.push_back(rc);

            // Reset element rects
            infoRect = { 0 };
            magnifierRect = { 0 };
        }

        // Draw info panel if needed
        if (needInfoUpdate) {
            // If already showing info, clear the old area first
            if (infoRect.right > infoRect.left) {
                FillRect(memDC, &infoRect, blackBrush);
                dirtyRects.push_back(infoRect);
            }

            // Draw new info panel
            infoRect = DrawInfo(memDC);

            // Mark as dirty if valid
            if (infoRect.right > infoRect.left && infoRect.bottom > infoRect.top) {
                dirtyRects.push_back(infoRect);
            }
        }

        // Draw magnifier if needed
        if (needMagnifierUpdate) {
            // If already showing magnifier, clear the old area first
            if (magnifierRect.right > magnifierRect.left) {
                FillRect(memDC, &magnifierRect, blackBrush);
                dirtyRects.push_back(magnifierRect);
            }

            // Draw new magnifier
            magnifierRect = DrawMagnifier(memDC);

            // Mark as dirty if valid
            if (magnifierRect.right > magnifierRect.left && magnifierRect.bottom > magnifierRect.top) {
                dirtyRects.push_back(magnifierRect);
            }
        }

        // Update the screen with all dirty rects
        for (const auto& rect : dirtyRects) {
            if (rect.right > rect.left && rect.bottom > rect.top) {
                BitBlt(hdc, rect.left, rect.top,
                    rect.right - rect.left,
                    rect.bottom - rect.top,
                    memDC, rect.left, rect.top, SRCCOPY);
            }
        }
    }

    // Clear the entire window before closing
    FillRect(memDC, &rc, blackBrush);
    BitBlt(hdc, 0, 0, screenWidth, screenHeight, memDC, 0, 0, SRCCOPY);

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
    int x_pos = 10;
    RECT boundingRect = { x_pos, y_pos, 350, y_pos }; // Increased width to prevent clipping

    // Get reference to settings
    bool aimbotEnabled = settings.aimbotData.enabled;
    bool weaponOffOverride = settings.activeState.weaponOffOverride;
    bool isPrimaryActive = settings.activeState.isPrimaryActive;
    int selectedCharacterIndex = settings.activeState.selectedCharacterIndex;

    // New indicators status
    bool quickPeekActive = settings.misc.hotkeys.IsActive(HotkeyIndex::AutoQuickPeek);
    bool hashomPeekActive = settings.misc.hotkeys.IsActive(HotkeyIndex::AutoHashomPeek);
    bool triggerBotActive = settings.misc.hotkeys.IsActive(HotkeyIndex::TriggerKey);
    bool uiHidden = settings.misc.hotkeys.IsActive(HotkeyIndex::HideUiKey);

    // Create a font with better readability
    HFONT newFont = CreateFont(
        18,                     // Height - slightly larger
        0,                      // Width
        0,                      // Escapement
        0,                      // Orientation
        FW_BOLD,                // Weight - BOLD for better visibility
        FALSE,                  // Italic
        FALSE,                  // Underline
        FALSE,                  // StrikeOut
        DEFAULT_CHARSET,        // CharSet
        OUT_DEFAULT_PRECIS,     // OutPrecision
        CLIP_DEFAULT_PRECIS,    // ClipPrecision
        ANTIALIASED_QUALITY,    // Quality - ANTIALIASED for smoother text
        DEFAULT_PITCH | FF_DONTCARE, // Pitch and Family
        L"Arial"                // Face Name
    );

    HFONT oldFont = (HFONT)SelectObject(hdc, newFont);

    // Function to draw text with a black background
    auto drawTextWithBackground = [&](const char* text, RECT& rect, COLORREF textColor) {
        // Create a solid black brush for the background
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));

        // Fill the background rectangle with black
        FillRect(hdc, &rect, blackBrush);

        // Draw the text with specified color
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, textColor);
        DrawTextA(hdc, text, -1, &rect, DT_LEFT);

        // Clean up
        DeleteObject(blackBrush);
        };

    // Title/header - add padding to rect
    RECT headerRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 }; // Increased height
    drawTextWithBackground(xorstr_("Focus Status"), headerRect, RGB(255, 255, 255));
    y_pos += 24;
    boundingRect.bottom = headerRect.bottom;

    // Separator line using GDI
    HPEN whitePen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    HPEN oldPen = (HPEN)SelectObject(hdc, whitePen);
    MoveToEx(hdc, x_pos, y_pos, NULL);
    LineTo(hdc, x_pos + 250, y_pos); // Wider separator
    SelectObject(hdc, oldPen);
    DeleteObject(whitePen);
    y_pos += 6; // More space

    // Draw aimbot status
    RECT aimbotRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 };
    std::string aimbotText = std::string(xorstr_("Aimbot: ")) + (aimbotEnabled ? xorstr_("ON") : xorstr_("OFF"));
    drawTextWithBackground(aimbotText.c_str(), aimbotRect, aimbotEnabled ? RGB(0, 255, 0) : RGB(255, 100, 100));
    boundingRect.bottom = aimbotRect.bottom;
    y_pos += 24;

    // Draw weapon status
    RECT weaponRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 };
    if (weaponOffOverride) {
        drawTextWithBackground(xorstr_("Weapon: DISABLED"), weaponRect, RGB(255, 100, 100));
    }
    else if (isPrimaryActive) {
        drawTextWithBackground(xorstr_("Weapon: PRIMARY"), weaponRect, RGB(0, 255, 0));
    }
    else {
        drawTextWithBackground(xorstr_("Weapon: SECONDARY"), weaponRect, RGB(0, 255, 0));
    }
    boundingRect.bottom = weaponRect.bottom;
    y_pos += 24;

    // Draw selected character
    if (selectedCharacterIndex < settings.characters.size() && !settings.characters.empty()) {
        RECT charRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 };
        std::string charText = std::string(xorstr_("Character: ")) +
            settings.characters[selectedCharacterIndex].charactername;
        drawTextWithBackground(charText.c_str(), charRect, RGB(255, 255, 255));
        boundingRect.bottom = charRect.bottom;
        y_pos += 24;
    }

    // Optionally show inference time for AI aimbot
    if (settings.aimbotData.type == 1 && aimbotEnabled) {
        RECT msRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 };
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

        drawTextWithBackground(ms_buffer.str().c_str(), msRect, timeColor);
        boundingRect.bottom = msRect.bottom;
        y_pos += 24;
    }

    // Separator for new indicators
    whitePen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
    oldPen = (HPEN)SelectObject(hdc, whitePen);
    MoveToEx(hdc, x_pos, y_pos, NULL);
    LineTo(hdc, x_pos + 250, y_pos);
    SelectObject(hdc, oldPen);
    DeleteObject(whitePen);
    y_pos += 6;

    // 1. Quick-peek/Hashom peek status with correct direction logic
    RECT peekRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 };
    std::string peekText;

    // Get current peek direction from Control class  
    if (quickPeekActive && hashomPeekActive) {
        // Both are active
        peekText = xorstr_("Peek: BOTH ACTIVE");
        if (settings.activeState.peekDirection == 1) {
            peekText += xorstr_(" (LEFT)");
        }
        else if (settings.activeState.peekDirection == 2) {
            peekText += xorstr_(" (RIGHT)");
        }
        else {
            peekText += xorstr_(" (READY)");
        }
        drawTextWithBackground(peekText.c_str(), peekRect, RGB(0, 255, 0));
    }
    else if (quickPeekActive) {
        peekText = xorstr_("Quick-Peek: ACTIVE");
        if (settings.activeState.peekDirection == 1) {
            peekText += xorstr_(" (LEFT)");
        }
        else if (settings.activeState.peekDirection == 2) {
            peekText += xorstr_(" (RIGHT)");
        }
        else {
            peekText += xorstr_(" (READY)");
        }
        drawTextWithBackground(peekText.c_str(), peekRect, RGB(0, 255, 0));
    }
    else if (hashomPeekActive) {
        peekText = xorstr_("Hashom-Peek: ACTIVE");
        if (settings.activeState.peekDirection == 1) {
            peekText += xorstr_(" (LEFT)");
        }
        else if (settings.activeState.peekDirection == 2) {
            peekText += xorstr_(" (RIGHT)");
        }
        else {
            peekText += xorstr_(" (READY)");
        }
        drawTextWithBackground(peekText.c_str(), peekRect, RGB(0, 255, 0));
    }
    else {
        peekText = xorstr_("Quick-Peek: OFF");
        drawTextWithBackground(peekText.c_str(), peekRect, RGB(255, 100, 100));
    }

    boundingRect.bottom = peekRect.bottom;
    y_pos += 24;

    // 2. Triggerbot status
    RECT triggerRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 };
    std::string triggerText = std::string(xorstr_("Triggerbot: ")) + (triggerBotActive ? xorstr_("ACTIVE") : xorstr_("OFF"));
    drawTextWithBackground(triggerText.c_str(), triggerRect, triggerBotActive ? RGB(0, 255, 0) : RGB(255, 100, 100));
    boundingRect.bottom = triggerRect.bottom;
    y_pos += 24;

    // 3. UI status (whether hidden)
    RECT uiRect = { x_pos, y_pos, boundingRect.right, y_pos + 24 };
    std::string uiText = std::string(xorstr_("Interface: ")) + (uiHidden ? xorstr_("HIDDEN") : xorstr_("VISIBLE"));
    drawTextWithBackground(uiText.c_str(), uiRect, uiHidden ? RGB(255, 165, 0) : RGB(0, 255, 0));
    boundingRect.bottom = uiRect.bottom;
    y_pos += 24;

    // Add some padding to the bounding rect
    boundingRect.right = x_pos + 280;  // Wider to prevent clipping
    boundingRect.bottom += 6;  // Add bottom padding

    // Clean up font
    SelectObject(hdc, oldFont);
    DeleteObject(newFont);

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
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
        FillRect(hdc, &magnifierRect, blackBrush);
        DeleteObject(blackBrush);
        return magnifierRect;
    }

    // Static buffers to reduce memory allocations
    static cv::Mat resizedMat;
    static bool bufferInitialized = false;
    static int lastSize = 0;
    static int lastSourceSize = 0;

    // If size changed, we need to reinitialize the buffers
    if (!bufferInitialized || lastSize != size || lastSourceSize != sourceSize) {
        // Pre-allocate the resized buffer with correct size
        resizedMat.create(size, size, CV_8UC4);
        bufferInitialized = true;
        lastSize = size;
        lastSourceSize = sourceSize;
    }

    // Get current desktop image safely with minimal lock time
    bool captureSucceeded = false;
    {
        // Minimize lock time by just checking if we can proceed
        if (globals.capture.desktopMutex_.try_lock()) {
            if (!globals.capture.desktopMat.empty()) {
                // Make sure source rectangle is within bounds
                sourceX = std::max(0, std::min(sourceX, globals.capture.desktopMat.cols - sourceSize));
                sourceY = std::max(0, std::min(sourceY, globals.capture.desktopMat.rows - sourceSize));
                sourceSize = std::min(sourceSize, std::min(globals.capture.desktopMat.cols - sourceX, globals.capture.desktopMat.rows - sourceY));

                if (sourceSize > 0) {
                    // Direct resizing without intermediate copying
                    cv::Rect roi(sourceX, sourceY, sourceSize, sourceSize);

                    // Use faster resize method with optimized parameters
                    cv::resize(globals.capture.desktopMat(roi), resizedMat, cv::Size(size, size),
                        0, 0, cv::INTER_NEAREST); // Use NEAREST for speed over quality

                    captureSucceeded = true;
                }
            }
            globals.capture.desktopMutex_.unlock();
        }
    }

    // Draw the magnifier - using more efficient GDI operations
    if (captureSucceeded) {
        // Create a temporary DIB section for direct memory transfer
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = resizedMat.cols;
        bmi.bmiHeader.biHeight = -resizedMat.rows; // Negative height for top-down DIB
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);

        if (hBitmap && bits) {
            // Copy data from OpenCV Mat to DIB section
            memcpy(bits, resizedMat.data, resizedMat.total() * resizedMat.elemSize());

            // Create temporary DC and select bitmap
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

            // Faster direct BitBlt instead of GDI+ DrawImage
            BitBlt(hdc, destX, destY, size, size, memDC, 0, 0, SRCCOPY);

            // Draw border - simpler and faster
            HPEN whitePen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
            HPEN oldPen = (HPEN)SelectObject(hdc, whitePen);
            HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, nullBrush);

            Rectangle(hdc, destX, destY, destX + size, destY + size);

            // Clean up GDI resources
            SelectObject(hdc, oldPen);
            DeleteObject(whitePen);
            SelectObject(hdc, oldBrush);

            // Clean up the temporary DC and bitmap
            SelectObject(memDC, oldBitmap);
            DeleteDC(memDC);
            DeleteObject(hBitmap);
        }
    }
    else {
        // If capture failed, just draw a border to indicate where magnifier would be
        HPEN redPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hdc, redPen);
        HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, blackBrush);

        // Fill with black and draw red border
        Rectangle(hdc, destX, destY, destX + size, destY + size);

        // Draw "X" in the center
        MoveToEx(hdc, destX, destY, NULL);
        LineTo(hdc, destX + size, destY + size);
        MoveToEx(hdc, destX, destY + size, NULL);
        LineTo(hdc, destX + size, destY);

        // Clean up
        SelectObject(hdc, oldPen);
        DeleteObject(redPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(blackBrush);
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