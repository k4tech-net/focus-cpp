#include "dxgi.hpp"

//Engine en;
Utils utils;

bool DXGI::InitDXGI() {
    // Create device and context
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &gDevice, &featureLevel, &gContext);
    if (FAILED(hr)) {
        std::cerr << xorstr_("Failed to create D3D11 device") << std::endl;
        return false;
    }

    // Get DXGI device
    IDXGIDevice* dxgiDevice = nullptr;
    hr = gDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (FAILED(hr)) {
        std::cerr << xorstr_("Failed to get DXGI device") << std::endl;
        return false;
    }

    // Get DXGI adapter
    IDXGIAdapter* dxgiAdapter = nullptr;
    hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter));
    if (FAILED(hr)) {
        dxgiDevice->Release();
        std::cerr << xorstr_("Failed to get DXGI adapter") << std::endl;
        return false;
    }

    // Find primary output (monitor)
    IDXGIOutput* dxgiOutput = nullptr;
    DXGI_OUTPUT_DESC outputDesc;
    bool foundPrimaryOutput = false;

    // Try to find the primary display adapter
    for (UINT i = 0; !foundPrimaryOutput; i++) {
        if (dxgiAdapter->EnumOutputs(i, &dxgiOutput) == DXGI_ERROR_NOT_FOUND) {
            break; // No more outputs
        }

        dxgiOutput->GetDesc(&outputDesc);
        if (outputDesc.AttachedToDesktop) {
            // Check if this is the primary monitor
            MONITORINFOEX monitorInfo;
            monitorInfo.cbSize = sizeof(MONITORINFOEX);
            if (GetMonitorInfo(outputDesc.Monitor, &monitorInfo) &&
                (monitorInfo.dwFlags & MONITORINFOF_PRIMARY)) {
                foundPrimaryOutput = true;
                break;
            }
        }

        dxgiOutput->Release();
        dxgiOutput = nullptr;
    }

    // If we couldn't find the primary output, fall back to the first output
    if (!foundPrimaryOutput) {
        if (FAILED(dxgiAdapter->EnumOutputs(0, &dxgiOutput))) {
            dxgiAdapter->Release();
            dxgiDevice->Release();
            std::cerr << xorstr_("Failed to get any DXGI output") << std::endl;
            return false;
        }
    }

    // Get output 1
    IDXGIOutput1* dxgiOutput1 = nullptr;
    hr = dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgiOutput1));
    if (FAILED(hr)) {
        dxgiOutput->Release();
        dxgiAdapter->Release();
        dxgiDevice->Release();
        std::cerr << xorstr_("Failed to get DXGI output1") << std::endl;
        return false;
    }

    // Create output duplication with retry logic for fullscreen apps
    for (int retryCount = 0; retryCount < 3; retryCount++) {
        hr = dxgiOutput1->DuplicateOutput(gDevice, &gOutputDuplication);

        if (SUCCEEDED(hr)) {
            break;
        }

        // If failed due to fullscreen app, wait a bit and retry
        if (hr == DXGI_ERROR_UNSUPPORTED) {
            std::cerr << xorstr_("Failed to duplicate output: DXGI_ERROR_UNSUPPORTED. Retrying...") << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        else {
            std::cerr << xorstr_("Failed to duplicate output, error code: ") << hr << std::endl;
            break;
        }
    }

    if (!gOutputDuplication) {
        dxgiOutput1->Release();
        dxgiOutput->Release();
        dxgiAdapter->Release();
        dxgiDevice->Release();
        std::cerr << xorstr_("Failed to create output duplication") << std::endl;
        return false;
    }

    // Cleanup intermediate interfaces
    dxgiOutput1->Release();
    dxgiOutput->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();

    return true;
}

void DXGI::CaptureDesktopDXGI() {
    // Track consecutive failures to help determine if we need to reinitialize
    int consecutiveFailures = 0;
    const int MAX_CONSECUTIVE_FAILURES = 60; // About 6 seconds at 10ms sleep

    while (!globals.shutdown.load()) {
        if (settings.misc.hotkeys.IsActive(HotkeyIndex::DisableKey)) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(500));
            continue;
        }

        IDXGIResource* desktopResource = nullptr;
        DXGI_OUTDUPL_FRAME_INFO frameInfo = {};

        // Use a less aggressive timeout
        HRESULT hr = E_FAIL;

        if (gOutputDuplication) {
            hr = gOutputDuplication->AcquireNextFrame(16, &frameInfo, &desktopResource);
        }
        else {
            // If duplication is null, we need to reinitialize
            consecutiveFailures = MAX_CONSECUTIVE_FAILURES;
        }

        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            // No new frame available, this is normal
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            consecutiveFailures = 0; // Reset counter since this is a normal condition
            continue;
        }
        else if (hr == DXGI_ERROR_ACCESS_LOST ||
            hr == DXGI_ERROR_UNSUPPORTED ||
            consecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
            // Common errors that occur with fullscreen apps:
            // - DXGI_ERROR_ACCESS_LOST: Desktop switch or resolution change
            // - DXGI_ERROR_UNSUPPORTED: App has exclusive fullscreen
            // - Too many consecutive failures: Something is persistently wrong

            std::cerr << xorstr_("Reinitializing DXGI due to errors or timeout") << std::endl;

            // Clean up existing resources
            if (gOutputDuplication) {
                gOutputDuplication->Release();
                gOutputDuplication = nullptr;
            }

            // Full reinitialization including device
            if (gContext) {
                gContext->Release();
                gContext = nullptr;
            }

            if (gDevice) {
                gDevice->Release();
                gDevice = nullptr;
            }

            // Try to completely reinitialize DXGI
            if (InitDXGI()) {
                std::cerr << xorstr_("DXGI reinitialization successful") << std::endl;
                consecutiveFailures = 0;
            }
            else {
                std::cerr << xorstr_("DXGI reinitialization failed, will retry") << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            continue;
        }
        else if (FAILED(hr)) {
            // Other error occurred
            consecutiveFailures++;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Reset failure counter when we successfully get a frame
        consecutiveFailures = 0;

        // Query for ID3D11Texture2D
        ID3D11Texture2D* desktopImageTex = nullptr;
        if (desktopResource) {
            hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&desktopImageTex));
            desktopResource->Release();
            desktopResource = nullptr;
        }

        if (FAILED(hr) || !desktopImageTex) {
            if (gOutputDuplication) {
                gOutputDuplication->ReleaseFrame();
            }
            consecutiveFailures++;
            continue;
        }

        // Get metadata to create Mat
        D3D11_TEXTURE2D_DESC desc;
        desktopImageTex->GetDesc(&desc);

        // Create staging texture with CPU read access
        ID3D11Texture2D* stagingTexture = nullptr;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;

        hr = gDevice->CreateTexture2D(&desc, nullptr, &stagingTexture);
        if (FAILED(hr) || !stagingTexture) {
            if (desktopImageTex) desktopImageTex->Release();
            if (gOutputDuplication) gOutputDuplication->ReleaseFrame();
            consecutiveFailures++;
            continue;
        }

        // Copy to staging texture
        gContext->CopyResource(stagingTexture, desktopImageTex);

        // Map resource
        D3D11_MAPPED_SUBRESOURCE resource;
        hr = gContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &resource);

        if (SUCCEEDED(hr) && resource.pData) {
            // Create OpenCV Mat from mapped data
            cv::Mat capturedFrame(desc.Height, desc.Width, CV_8UC4, resource.pData, resource.RowPitch);

            // Make a deep copy to ensure we have the data after unmapping
            cv::Mat desktopImage = capturedFrame.clone();

            // Unmap and release D3D resources before handling the OpenCV processing
            gContext->Unmap(stagingTexture, 0);
            stagingTexture->Release();
            desktopImageTex->Release();

            if (gOutputDuplication) {
                gOutputDuplication->ReleaseFrame();
            }

            // Validate the frame before passing it on
            if (!desktopImage.empty() && desktopImage.data &&
                desktopImage.cols > 0 && desktopImage.rows > 0) {

                // Swap buffers with lock
                globals.capture.desktopMutex_.lock();
                globals.capture.desktopMat = desktopImage;
                globals.capture.desktopMutex_.unlock();

                if (!globals.capture.initDims.load()) {
                    globals.capture.initDims.store(true);
                    globals.capture.desktopWidth.store(desktopImage.cols);
                    globals.capture.desktopHeight.store(desktopImage.rows);
                    globals.capture.desktopCenterX.store(desktopImage.cols / 2);
                    globals.capture.desktopCenterY.store(desktopImage.rows / 2);
                }
            }
        }
        else {
            // Failed to map texture
            stagingTexture->Release();
            desktopImageTex->Release();
            gOutputDuplication->ReleaseFrame();
            consecutiveFailures++;
        }

        // Small sleep to yield CPU to other threads
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    // Cleanup on exit
    CleanupDXGI();
}

void DXGI::CleanupDXGI() {
    if (gOutputDuplication) {
        gOutputDuplication->Release();
    }
    if (gContext) {
        gContext->Release();
    }
    if (gDevice) {
        gDevice->Release();
    }
}

std::vector<float> calculateCorrections(const cv::Mat& image, const std::vector<Detection>& detections, int targetClass, float fov, bool forceHitbox) {
    float imageCenterX = image.cols / 2.f;
    float imageCenterY = image.rows / 2.f;

    float minDistance = std::numeric_limits<float>::max();
    cv::Point2f closestDetectionCenter(imageCenterX, imageCenterY);
    bool found = false;

    for (const auto& detection : detections) {
        // Original logic when forceHitbox is true, or when targeting closest (2)
        if (targetClass == 2 || (forceHitbox && detection.class_id == targetClass) ||
            (!forceHitbox && (detection.class_id == targetClass || (!found && (detection.class_id == 0 || detection.class_id == 1))))) {

            cv::Point2f detectionCenter(detection.box.x + detection.box.width / 2.f,
                detection.box.y + detection.box.height / 2.f);
            float distance = static_cast<float>(cv::norm(detectionCenter - cv::Point2f(imageCenterX, imageCenterY)));

            if (distance < minDistance && (targetClass == 2 || forceHitbox || detection.class_id == targetClass || !found)) {
                minDistance = distance;
                closestDetectionCenter = detectionCenter;
                found = true;
            }
        }
    }


    if (!found) {
        return { 0.0f, 0.0f };
    }

    // Calculate pixel offsets from center
    float pixelOffsetX = closestDetectionCenter.x - imageCenterX;
    float pixelOffsetY = closestDetectionCenter.y - imageCenterY;

    // Convert FOV from degrees to radians
    float fovRad = static_cast<float>(fov * std::numbers::pi / 180.0f);

    // Calculate the number of pixels per degree
    // This uses half the screen width since FOV is horizontal
    float pixelsPerDegree = (float)image.cols / fov;

    // Convert pixel offsets to angles (in degrees)
    float angleX = pixelOffsetX / pixelsPerDegree;
    float angleY = pixelOffsetY / pixelsPerDegree;

    // Calculate vertical FOV based on aspect ratio
    float aspectRatio = (float)image.cols / image.rows;
    float verticalFov = static_cast<float>(2.0f * atan(tan(fovRad / 2.0f) / aspectRatio) * 180.0f / std::numbers::pi);

    // Scale Y angle based on vertical FOV
    angleY = angleY * (fov / verticalFov);

	float mouseX = 0.0f;
	float mouseY = 0.0f;

    // Convert angles to mouse input where 7274 = 360 degrees
    switch (settings.globalSettings.sensitivityCalculator) {
    case 0:
		mouseX = (angleX / 360.0f) * constants.SIEGE360DIST;
		mouseY = (angleY / 360.0f) * constants.SIEGE360DIST;
		break;
	case 1:
		mouseX = (angleX / 360.0f) * constants.SIEGE360DIST;
		mouseY = (angleY / 360.0f) * constants.SIEGE360DIST;
		break;
	case 2:
		mouseX = (angleX / 360.0f) * constants.RUST360DIST;
		mouseY = (angleY / 360.0f) * constants.RUST360DIST;
		break;
	case 3:
		mouseX = (angleX / 360.0f) * constants.OW360DIST;
		mouseY = (angleY / 360.0f) * constants.OW360DIST;
		break;
    }

    mouseX *= settings.activeState.fovSensitivityModifier;
    mouseY *= settings.activeState.fovSensitivityModifier;

    return { mouseX, mouseY };
}

cv::Mat DXGI::normalizeIconSize(const cv::Mat& icon, int width, int height) {
    cv::Mat resized;
    cv::resize(icon, resized, cv::Size(width, height), 0, 0, cv::INTER_AREA);
    return resized;
}

cv::Mat DXGI::preprocessIcon(const cv::Mat& icon) {
    cv::Mat gray;
    cv::cvtColor(icon, gray, cv::COLOR_BGR2GRAY);

    // Remove potential defuser icon area first
    int defuserSize = static_cast<int>(gray.rows / 2.5f);  // Assume defuser icon is about 1/4 of the icon height
    cv::rectangle(gray, cv::Rect(gray.cols - defuserSize, gray.rows - defuserSize, defuserSize, defuserSize), cv::Scalar(0), cv::FILLED);

    // Then apply adaptive thresholding to handle brightness variations
    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 2);

    return binary;
}

IconHash DXGI::hashIcon(const cv::Mat& icon, int size) {
    cv::Mat preprocessed = preprocessIcon(icon);
    IconHash hash;

    int idx = 0;
    for (int y = 0; y < preprocessed.rows; y += size) {
        for (int x = 0; x < preprocessed.cols; x += size) {
            cv::Rect block(x, y, size, size);
            double avgIntensity = cv::mean(preprocessed(block))[0];
            hash[idx++] = (avgIntensity > 127);

            // Calculate horizontal gradient
            if (x < preprocessed.cols - size) {
                cv::Rect nextBlock(x + size, y, size, size);
                double nextAvgIntensity = cv::mean(preprocessed(nextBlock))[0];
                hash[idx++] = (nextAvgIntensity > avgIntensity);
            }
        }
    }

    return hash;
}

void DXGI::aimbot() {

    std::wstring modelPath = xorstr_(L"focus.onnx");
    const char* logid = xorstr_("yolo_inference");
    const char* provider;

    bool aimbotInit = false;

    const float cropRatioHeight = 0.33f;

    std::unique_ptr<YoloInferencer> inferencer;

    while (!globals.shutdown.load()) {
        if (settings.misc.hotkeys.IsActive(HotkeyIndex::DisableKey)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        if (!settings.aimbotData.enabled) {
            if (aimbotInit) {
                inferencer.reset();

                if (cudaDeviceReset() != cudaSuccess) {
                    std::cerr << xorstr_("Warning: Failed to reset CUDA device") << std::endl;
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(50));

                aimbotInit = false;
            }
            settings.aimbotData.correctionX = 0;
            settings.aimbotData.correctionY = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        if (settings.aimbotData.type == 0 && settings.aimbotData.enabled) {
            if (globals.capture.desktopMat.empty()) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                continue;
            }

            cv::Mat croppedImage;
            {
                std::lock_guard<std::mutex> lock(globals.capture.desktopMutex_);

                // Calculate square ROI based on height (33%)
                const int screenWidth = globals.capture.desktopMat.cols;
                const int screenHeight = globals.capture.desktopMat.rows;

                // Height is 33% of screen height
                const int roiHeight = static_cast<int>(cropRatioHeight * screenHeight);

                // Width equals height for square aspect ratio
                const int roiWidth = roiHeight;

                // Center the square in the screen
                const int roiX = (screenWidth - roiWidth) / 2;
                const int roiY = (screenHeight - roiHeight) / 2;

                const cv::Rect roi(roiX, roiY, roiWidth, roiHeight);

                // Actually crop the image (this was missing in original code)
                croppedImage = globals.capture.desktopMat(roi).clone();
            }

            if (croppedImage.empty()) {
                continue;
            }

            overwatchDetector(croppedImage);

            if (settings.aimbotData.limitDetectorFps) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
        else if (settings.aimbotData.type == 1 && settings.aimbotData.enabled) {
            if (!aimbotInit) {
                switch (settings.aimbotData.aiAimbotSettings.provider) {
                    case 0:
                        provider = xorstr_("CPU");
                        break;
                    case 1:
                        provider = xorstr_("CUDA");
                        break;
                    default:
                        provider = xorstr_("TensorRT");
                        break;
                }

                inferencer = std::make_unique<YoloInferencer>(modelPath, logid, provider);
                aimbotInit = true;
            }

            if (!aimbotInit) {
                continue;
            }

            cv::Mat croppedImage;
            {
                std::lock_guard<std::mutex> lock(globals.capture.desktopMutex_);
                if (globals.capture.desktopMat.empty()) {
                    continue;
                }

                // Calculate square ROI based on height (33%)
                const int screenWidth = globals.capture.desktopMat.cols;
                const int screenHeight = globals.capture.desktopMat.rows;

                // Height is 33% of screen height
                const int roiHeight = static_cast<int>(cropRatioHeight * screenHeight);

                // Width equals height for square aspect ratio
                const int roiWidth = roiHeight;

                // Center the square in the screen
                const int roiX = (screenWidth - roiWidth) / 2;
                const int roiY = (screenHeight - roiHeight) / 2;

                const cv::Rect roi(roiX, roiY, roiWidth, roiHeight);

                // Convert BGRA to BGR during the crop operation
                cv::cvtColor(globals.capture.desktopMat(roi), croppedImage, cv::COLOR_BGRA2BGR);
            }

            if (croppedImage.empty()) {
                continue;
            }

            std::vector<Detection> detections = inferencer->infer(croppedImage, PERCENT(settings.aimbotData.aiAimbotSettings.confidence), 0.5);

            if (detections.empty()) {
                settings.aimbotData.correctionX = 0;
                settings.aimbotData.correctionY = 0;
                continue;
            }

            // draw detections on the image
			for (const auto& detection : detections) {
				cv::rectangle(croppedImage, detection.box, cv::Scalar(0, 255, 0), 2);
			}

            std::vector<float> corrections = calculateCorrections(croppedImage, detections, settings.aimbotData.aiAimbotSettings.hitbox, settings.globalSettings.fov, settings.aimbotData.aiAimbotSettings.forceHitbox);

            settings.aimbotData.correctionX = static_cast<int>(corrections[0] * settings.activeState.sensMultiplier_SensOnly[0]);
            settings.aimbotData.correctionY = static_cast<int>(corrections[1] * settings.activeState.sensMultiplier_SensOnly[1]);

            if (settings.aimbotData.limitDetectorFps) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    // Ensure clean shutdown
    inferencer.reset();
    cudaDeviceReset();
}

void DXGI::triggerbot() {
    // Static buffers to avoid reallocations
    static cv::Mat prevFrame, currentFrame, diffFrame;
    static bool firstFrame = true;
    static std::chrono::steady_clock::time_point lastTriggerTime = std::chrono::steady_clock::now();
    static bool isBurstActive = false;
    static std::chrono::steady_clock::time_point burstStartTime;
    static bool debugWindowCreated = false;
    static bool justFired = false;
    static cv::Rect roi;
    static int lastRadius = -1;
    static bool currentWeaponHasRapidfire = false;

    while (!globals.shutdown.load()) {
        if (settings.misc.hotkeys.IsActive(HotkeyIndex::DisableKey)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        // Early exit if trigger key not active
        if (!settings.misc.hotkeys.IsActive(HotkeyIndex::TriggerKey)) {
            if (debugWindowCreated) {
                cv::destroyWindow(xorstr_("TriggerBot Detection"));
                debugWindowCreated = false;
            }

            if (isBurstActive) {
                utils.pressMouse1(false);
                isBurstActive = false;
            }

            firstFrame = true;
            justFired = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        // Cache weapon details once per iteration for performance
        const characterData& currentCharacter = settings.characters[settings.activeState.selectedCharacterIndex];
        const bool isPrimaryActive = settings.activeState.isPrimaryActive;
        const int weaponIndex = isPrimaryActive ?
            currentCharacter.selectedweapon[0] :
            currentCharacter.selectedweapon[1];
        const weaponData& currentWeapon = currentCharacter.weapondata[weaponIndex];

        currentWeaponHasRapidfire = currentWeapon.rapidfire;

        // Handle timing and state
        auto currentTime = std::chrono::steady_clock::now();

        // Process burst timing
        if (isBurstActive) {
            auto burstElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - burstStartTime).count();

            if (currentWeaponHasRapidfire) {
                // For rapidfire weapons, toggle the mouse state during burst
                static bool buttonState = true;
                static std::chrono::steady_clock::time_point lastToggleTime = burstStartTime;

                auto timeSinceToggle = std::chrono::duration_cast<std::chrono::milliseconds>(
                    currentTime - lastToggleTime).count();

                // Toggle at a fixed rate for semi-auto weapons
                if (timeSinceToggle >= 30) {  // 30ms between toggles
                    buttonState = !buttonState;
                    utils.pressMouse1(buttonState);
                    lastToggleTime = currentTime;
                }

                // End burst when duration is reached
                if (burstElapsed >= currentWeapon.triggerBurstDuration) {
                    utils.pressMouse1(false);
                    isBurstActive = false;
                    justFired = true;
                    firstFrame = true;
                }
            }
            else {
                // Standard burst for regular weapons
                if (burstElapsed >= currentWeapon.triggerBurstDuration) {
                    utils.pressMouse1(false);
                    isBurstActive = false;
                    justFired = true;
                    firstFrame = true;
                }
            }

            if (isBurstActive) {
                std::this_thread::sleep_for(std::chrono::microseconds(200));
                continue;
            }
        }

        // Check cooldown state
        auto timeSinceLastTrigger = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTriggerTime).count();
        bool inCooldown = timeSinceLastTrigger < settings.aimbotData.triggerSettings.sleepTime;

        // Reset after cooldown
        if (justFired && !inCooldown) {
            firstFrame = true;
            justFired = false;
        }

        // Skip processing during cooldown
        if (inCooldown) {
            std::this_thread::sleep_for(std::chrono::microseconds(200));
            continue;
        }

        // Get trigger settings (outside of lock)
        const int radius = settings.aimbotData.triggerSettings.radius;
        const float sensitivity = settings.aimbotData.triggerSettings.sensitivity;
        const int detectionMethod = settings.aimbotData.triggerSettings.detectionMethod;

        // Only recalculate ROI if radius changed
        int desktopWidth = globals.capture.desktopWidth.load();
        int desktopHeight = globals.capture.desktopHeight.load();

        if (lastRadius != radius) {
            const int centerX = desktopWidth / 2;
            const int centerY = desktopHeight / 2;
            roi = cv::Rect(
                std::max(0, centerX - radius),
                std::max(0, centerY - radius),
                std::min(radius * 2, desktopWidth - (centerX - radius)),
                std::min(radius * 2, desktopHeight - (centerY - radius))
            );
            lastRadius = radius;
        }

        // Capture with minimal lock time
        {
            std::lock_guard<std::mutex> lock(globals.capture.desktopMutex_);
            if (globals.capture.desktopMat.empty()) {
                std::this_thread::sleep_for(std::chrono::microseconds(200));
                continue;
            }

            // Extract ROI directly to our static buffer
            globals.capture.desktopMat(roi).copyTo(currentFrame);
        }

        if (currentFrame.empty()) {
            std::this_thread::sleep_for(std::chrono::microseconds(200));
            continue;
        }

        // Minimal color conversion only if needed
        if (currentFrame.channels() == 4) {
            cv::cvtColor(currentFrame, currentFrame, cv::COLOR_BGRA2BGR);
        }

        // First frame initialization
        if (firstFrame) {
            currentFrame.copyTo(prevFrame);
            firstFrame = false;
            std::this_thread::sleep_for(std::chrono::microseconds(200));
            continue;
        }

        // Ensure correct size for comparison
        if (prevFrame.size() != currentFrame.size()) {
            currentFrame.copyTo(prevFrame);
            std::this_thread::sleep_for(std::chrono::microseconds(200));
            continue;
        }

        // Fast detection with early stopping
        bool shouldTrigger = false;
        double change = 0.0;

        // Optimize detection order: start with the fastest method first
        // Early stopping: if one detection method succeeds, skip others
        if (detectionMethod == 0 || detectionMethod == 2) { // Color change
            cv::Scalar prevMean = cv::mean(prevFrame);
            cv::Scalar currentMean = cv::mean(currentFrame);

            double colorChange = 0.0;
            for (int i = 0; i < 3; i++) {
                colorChange += std::abs(currentMean[i] - prevMean[i]);
            }
            colorChange /= 3.0;
            change = colorChange;

            // Early detection success
            if (change > sensitivity) {
                shouldTrigger = true;
                // Skip motion detection if we've already detected change
                if (detectionMethod == 0) {
                    goto triggerDecision;
                }
            }
        }

        if (!shouldTrigger && (detectionMethod == 1 || detectionMethod == 2)) { // Motion detection
            cv::absdiff(currentFrame, prevFrame, diffFrame);
            cv::Scalar diffMean = cv::mean(diffFrame);

            double motionChange = (diffMean[0] + diffMean[1] + diffMean[2]) / 3.0;
            change = std::max(change, motionChange);

            if (change > sensitivity) {
                shouldTrigger = true;
            }
        }

    triggerDecision:
        // Trigger decision with minimal branching
        if (shouldTrigger && !inCooldown && !justFired) {
            bool mouseAlreadyDown = globals.mouseinfo.l_mouse_down.load(std::memory_order_relaxed);

            if (!mouseAlreadyDown && !isBurstActive) {
                auto preDelayTime = std::chrono::steady_clock::now();

                if (currentWeapon.triggerFireDelay > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(currentWeapon.triggerFireDelay));

                    // Check if target is still valid after delay (recheck shouldTrigger condition)
                    if (!settings.misc.hotkeys.IsActive(HotkeyIndex::TriggerKey)) {
                        // User released trigger key during delay
                        continue;
                    }
                }

                if (currentWeapon.triggerBurstDuration > 0) {
                    // Burst mode
                    utils.pressMouse1(true);
                    isBurstActive = true;
                    burstStartTime = std::chrono::steady_clock::now();
                }
                else {
                    // Single click mode - optimized for minimum latency
                    utils.pressMouse1(true);
                    std::this_thread::sleep_for(std::chrono::microseconds(300));
                    utils.pressMouse1(false);
                    justFired = true;
                    firstFrame = true;
                }

                lastTriggerTime = preDelayTime;
            }
        }

        // Debug visualization (only if enabled)
        if (settings.aimbotData.triggerSettings.showDebug) {
            cv::Mat debugFrame = currentFrame.clone();

            std::string changeText = xorstr_("Change: ") + std::to_string(change);
            cv::putText(debugFrame, changeText, cv::Point(5, 15), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                change > sensitivity ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0), 1);

            cv::imshow(xorstr_("TriggerBot Detection"), debugFrame);
            cv::waitKey(1);
            debugWindowCreated = true;
        }
        else if (debugWindowCreated) {
            cv::destroyWindow(xorstr_("TriggerBot Detection"));
            debugWindowCreated = false;
        }

        // Update previous frame - use copyTo for better performance
        currentFrame.copyTo(prevFrame);

        // Minimal sleep to allow other threads to run
        if (settings.aimbotData.limitDetectorFps) {
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        else {
            std::this_thread::sleep_for(std::chrono::microseconds(200)); // ~5000 FPS
        }
    }
}

void DXGI::detectWeaponR6(cv::Mat& src, double hysteresisThreshold, double minActiveAreaThreshold) {

    if (src.empty()) {
        return;
    }

    // Define the region of interest (ROI) coordinates as a percentage of the frame
    float roi1XPercent = 0.f; // X coordinate percentage for ROI 1
    float roi1YPercent = 0.06f; // Y coordinate percentage for ROI 1
    float roi1WidthPercent = 1.f; // Width percentage for ROI 1
    float roi1HeightPercent = 0.25f; // Height percentage for ROI 1

    float roi2XPercent = 0.f; // X coordinate percentage for ROI 2
    float roi2YPercent = 0.4f; // Y coordinate percentage for ROI 2
    float roi2WidthPercent = 1.f; // Width percentage for ROI 2
    float roi2HeightPercent = 0.25f; // Height percentage for ROI 2

    float roi3XPercent = 0.f; // X coordinate percentage for ROI 3
    float roi3YPercent = 0.7f; // Y coordinate percentage for ROI 3
    float roi3WidthPercent = 1.f; // Width percentage for ROI 3
    float roi3HeightPercent = 0.25f; // Height percentage for ROI 3

    // Calculate ROI coordinates based on percentage of frame dimensions
    cv::Rect roi1(static_cast<int>(src.cols * roi1XPercent),
        static_cast<int>(src.rows * roi1YPercent),
        static_cast<int>(src.cols * roi1WidthPercent),
        static_cast<int>(src.rows * roi1HeightPercent));

    cv::Rect roi2(static_cast<int>(src.cols * roi2XPercent),
        static_cast<int>(src.rows * roi2YPercent),
        static_cast<int>(src.cols * roi2WidthPercent),
        static_cast<int>(src.rows * roi2HeightPercent));

    cv::Rect roi3(static_cast<int>(src.cols * roi3XPercent),
        static_cast<int>(src.rows * roi3YPercent),
        static_cast<int>(src.cols * roi3WidthPercent),
        static_cast<int>(src.rows * roi3HeightPercent));

    // Ensure the ROIs are within the bounds of the source image
    if ((roi1 & cv::Rect(0, 0, src.cols, src.rows)) != roi1 ||
        (roi2 & cv::Rect(0, 0, src.cols, src.rows)) != roi2 ||
        (roi3 & cv::Rect(0, 0, src.cols, src.rows)) != roi3) {
        return;
    }

    // Highlight the ROIs on the source image for alignment
    rectangle(src, roi1, cv::Scalar(255, 0, 0), 2); // Blue rectangle around ROI 1
    rectangle(src, roi2, cv::Scalar(255, 0, 0), 2); // Blue rectangle around ROI 2
    rectangle(src, roi3, cv::Scalar(255, 0, 0), 2); // Blue rectangle around ROI 3

    // Extract the ROIs from the source image
    cv::Mat roiImg1 = src(roi1);
    cv::Mat roiImg2 = src(roi2);
    cv::Mat roiImg3 = src(roi3);

    // Convert ROIs to RGB for processing
    cv::Mat rgb1, rgb2, rgb3;
    cvtColor(roiImg1, rgb1, cv::COLOR_BGR2RGB);
    cvtColor(roiImg2, rgb2, cv::COLOR_BGR2RGB);
    cvtColor(roiImg3, rgb3, cv::COLOR_BGR2RGB);

    cv::Vec3b primaryTargetColour = cv::Vec3b(15, 255, 243);
    int primaryBuffer = 15;

    double primaryArea1 = 0, primaryArea2 = 0, primaryArea3 = 0;
    double totalBrightness1 = 0, totalBrightness2 = 0, totalBrightness3 = 0;

    // Calculate the total area of matching color and total brightness in each ROI
    for (int y = 0; y < rgb1.rows; ++y) {
        for (int x = 0; x < rgb1.cols; ++x) {
            cv::Vec3b pixel = rgb1.at<cv::Vec3b>(y, x);
            if (pixel[0] >= primaryTargetColour[0] - primaryBuffer && pixel[0] <= primaryTargetColour[0] + primaryBuffer &&
                pixel[1] >= primaryTargetColour[1] - primaryBuffer && pixel[1] <= primaryTargetColour[1] + primaryBuffer &&
                pixel[2] >= primaryTargetColour[2] - primaryBuffer && pixel[2] <= primaryTargetColour[2] + primaryBuffer) {
                primaryArea1++;
            }
            double brightness = 0.2126 * pixel[2] + 0.7152 * pixel[1] + 0.0722 * pixel[0]; // Calculate the brightness
            totalBrightness1 += brightness;
        }
    }
    for (int y = 0; y < rgb2.rows; ++y) {
        for (int x = 0; x < rgb2.cols; ++x) {
            cv::Vec3b pixel = rgb2.at<cv::Vec3b>(y, x);
            if (pixel[0] >= primaryTargetColour[0] - primaryBuffer && pixel[0] <= primaryTargetColour[0] + primaryBuffer &&
                pixel[1] >= primaryTargetColour[1] - primaryBuffer && pixel[1] <= primaryTargetColour[1] + primaryBuffer &&
                pixel[2] >= primaryTargetColour[2] - primaryBuffer && pixel[2] <= primaryTargetColour[2] + primaryBuffer) {
                primaryArea2++;
            }
            double brightness = 0.2126 * pixel[2] + 0.7152 * pixel[1] + 0.0722 * pixel[0]; // Calculate the brightness
            totalBrightness2 += brightness;
        }
    }
    for (int y = 0; y < rgb3.rows; ++y) {
        for (int x = 0; x < rgb3.cols; ++x) {
            cv::Vec3b pixel = rgb3.at<cv::Vec3b>(y, x);
            if (pixel[0] >= primaryTargetColour[0] - primaryBuffer && pixel[0] <= primaryTargetColour[0] + primaryBuffer &&
                pixel[1] >= primaryTargetColour[1] - primaryBuffer && pixel[1] <= primaryTargetColour[1] + primaryBuffer &&
                pixel[2] >= primaryTargetColour[2] - primaryBuffer && pixel[2] <= primaryTargetColour[2] + primaryBuffer) {
                primaryArea3++;
            }
            double brightness = 0.2126 * pixel[2] + 0.7152 * pixel[1] + 0.0722 * pixel[0]; // Calculate the brightness
            totalBrightness3 += brightness;
        }
    }

    // Apply hysteresis to prevent rapid changes in ROIs
    if (abs(primaryArea1 - prevPrimaryArea1) < hysteresisThreshold) {
        primaryArea1 = prevPrimaryArea1;
    }
    if (abs(primaryArea2 - prevPrimaryArea2) < hysteresisThreshold) {
        primaryArea2 = prevPrimaryArea2;
    }
    if (abs(primaryArea3 - prevPrimaryArea3) < hysteresisThreshold) {
        primaryArea3 = prevPrimaryArea3;
    }

    // Update previous area values
    prevPrimaryArea1 = primaryArea1;
    prevPrimaryArea2 = primaryArea2;
    prevPrimaryArea3 = primaryArea3;

    int activeROI = 0;

    if (settings.activeState.selectedCharacterIndex >= settings.characters.size() ||
        settings.characters.empty()) {
        settings.activeState.weaponOffOverride = true;
        return;
    }

    if ((settings.characters[settings.activeState.selectedCharacterIndex].options.size() > 0) &&
        settings.characters[settings.activeState.selectedCharacterIndex].options[0] &&
        primaryArea1 > minActiveAreaThreshold && primaryArea1 > primaryArea2 && primaryArea1 > primaryArea3) {
    
        if (totalBrightness2 > minActiveAreaThreshold && totalBrightness2 > totalBrightness3) {
            activeROI = 2;
            rectangle(src, roi2, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 2
        }
        else if (totalBrightness3 > minActiveAreaThreshold && totalBrightness3 > totalBrightness2) {
    		activeROI = 3;
    		rectangle(src, roi3, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 3
    	}
    }
    else {
        if (primaryArea1 > minActiveAreaThreshold && primaryArea1 > primaryArea2 && primaryArea1 > primaryArea3) {
            activeROI = 1;
            rectangle(src, roi1, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 1
        }
        else if (primaryArea2 > minActiveAreaThreshold && primaryArea2 > primaryArea1 && primaryArea2 > primaryArea3) {
            activeROI = 2;
            rectangle(src, roi2, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 2
        }
        else if (primaryArea3 > minActiveAreaThreshold && primaryArea3 > primaryArea1 && primaryArea3 > primaryArea2) {
            activeROI = 3;
            rectangle(src, roi3, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 3
        }
    }

    // Print the index of the active ROI or indicate neither if both are not active
    if (activeROI == 0 || activeROI == 1) {
        //std::cout << "Neither ROI is active" << std::endl;
        settings.activeState.weaponOffOverride = true;
    }
    else {
        //std::cout << "Active ROI: " << activeROI << std::endl;
        settings.activeState.weaponOffOverride = false;
        if (activeROI == 2) {
            settings.activeState.isPrimaryActive = true;
            settings.activeState.weaponDataChanged = true;
        }
        else if (activeROI == 3) {
            settings.activeState.isPrimaryActive = false;
            settings.activeState.weaponDataChanged = true;
        }
    }
}

void debugHammingDistances(const IconHash& hash, const std::unordered_map<IconHash, std::string>& hashset) {
    std::vector<std::pair<std::string, float>> distances;

    for (const auto& pair : hashset) {
        int distance = utils.hammingDistance(hash, pair.first);
        float percentage = static_cast<float>(distance) / HASH_SIZE * 100.0f;
        distances.push_back({ pair.second, percentage });
    }

    // Sort by distance
    std::sort(distances.begin(), distances.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    // Print top 5 closest matches
    std::cout << xorstr_("Top 5 closest matches:\n");
    for (size_t i = 0; i < std::min(size_t(5), distances.size()); ++i) {
        std::cout << distances[i].first << xorstr_(": ")
            << distances[i].second << xorstr_("% different\n");
    }
}

bool DXGI::detectOperatorR6(cv::Mat& src) {

    if (src.empty()) {
        return false;
    }

    cv::Mat normalizedIcon = normalizeIconSize(src, 64, 64);
    IconHash hash = hashIcon(normalizedIcon, 4);

    //std::cout << xorstr_("Hash: ") << hash << std::endl;
	//debugHammingDistances(hash, operatorHashes);

    std::string detectedOperator;
    bool operatorFound = false;

    // Compare the hash with pre-computed hashes for known operators
    auto it = operatorHashes.find(hash);
    if (it != operatorHashes.end()) {
        detectedOperator = it->second;
        operatorFound = true;
    }
    else {
        // No exact match, find the closest match
        int minHammingDistance = HASH_SIZE;  // Maximum possible Hamming distance for a 64-bit hash
        float bestMatchPercentage = 100.0f;

        for (const auto& pair : operatorHashes) {
            int distance = utils.hammingDistance(hash, pair.first);
            float percentDiff = static_cast<float>(distance) / HASH_SIZE * 100.0f;

            if (distance < minHammingDistance) {
                minHammingDistance = distance;
                bestMatchPercentage = percentDiff;
                detectedOperator = pair.second;
            }
        }

        if (bestMatchPercentage <= 10.f) {  // Adjust between 1-5% based on testing
            operatorFound = true;
        }
    }

    if (operatorFound) {
        int characterIndex = utils.findCharacterIndex(detectedOperator);
        if (characterIndex != -1) {
            settings.activeState.selectedCharacterIndex = characterIndex;
            settings.activeState.weaponDataChanged = true;
			return true;
        }
    }

    return false;
}

std::vector<int> DXGI::detectAttachmentsR6FromRegion(cv::Mat& attachmentRegion) {
    if (attachmentRegion.empty()) {
        return { 0, 0, 0 }; // Default attachments: [scope, grip, barrel]
    }

    std::vector<cv::Rect> attachmentRois;
    float iconHeight = 0.25f;
    float iconWidth = 1.f;
    float iconX = 0.f;

    // Define ROIs for different attachment types
    float scopeY = 0.f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(scopeY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    float barrelY = 0.25f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(barrelY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    float gripY = 0.50f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(gripY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    float laserY = 0.75f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(laserY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    std::vector<std::string> detectedAttachments = { "", "", "", "" }; // Defaults: [scope, barrel, grip, laser]

    // Process each attachment region
    for (int i = 0; i < attachmentRois.size(); i++) {
        cv::Mat attachment = attachmentRegion(attachmentRois[i]);
        cv::Mat normalizedAttachment = normalizeIconSize(attachment, 32, 32);

        IconHash attachmentHash = hashIcon(normalizedAttachment, 2);

        //std::cout << "Attachment " << i << " hash: " << attachmentHash << std::endl;
        //debugHammingDistances(attachmentHash, attachmentHashes);

        // Try exact hash match first
        auto it = attachmentHashes.find(attachmentHash);
        if (it != attachmentHashes.end()) {
            //std::cout << "Attachment " << i << ": " << it->second << std::endl;
            detectedAttachments[i] = it->second;
        }
        else {
            // Try approximate match with Hamming distance
            int minHammingDistance = HASH_SIZE;
            std::string bestMatch = "";

            for (const auto& pair : attachmentHashes) {
                int distance = utils.hammingDistance(attachmentHash, pair.first);

                if (distance < minHammingDistance) {
                    minHammingDistance = distance;
                    bestMatch = pair.second;
                }
            }

            float percentDiff = static_cast<float>(minHammingDistance) / HASH_SIZE * 100.0f;
            if (percentDiff <= 15.0f) {
                detectedAttachments[i] = bestMatch;
            }

            //std::cout << "Attachment " << i << ": " << bestMatch << " (" << percentDiff << "%)" << std::endl;
        }

        // Draw debug rectangles
        cv::rectangle(attachmentRegion, attachmentRois[i], cv::Scalar(0, 255, 0), 1);
    }

    // Map detected attachments to their correct indices: [scope, grip, barrel]
    std::vector<int> attachmentIndices = { 0, 0, 0 };

    // Process scope (index 0)
    if (!detectedAttachments[0].empty()) {
        std::string scope = detectedAttachments[0];

        if (scope.find("Telescopic") != std::string::npos) {
            attachmentIndices[0] = 2; // 3.5x scope
        }
        else if (scope.find("Magnified") != std::string::npos) {
            attachmentIndices[0] = 1; // 2.5x scope
        }
        else if (scope.find("Red_Dot") != std::string::npos ||
            scope.find("Holo") != std::string::npos ||
            scope.find("Reflex") != std::string::npos ||
            scope.find("Iron_Sight") != std::string::npos) {
            attachmentIndices[0] = 0; // 1x scope
        }
    }

    // Process barrel (index 2 in vector, but position 1 in UI)
    if (!detectedAttachments[1].empty()) {
        std::string barrel = detectedAttachments[1];

        if (barrel.find("Suppressor") != std::string::npos ||
            barrel.find("Extended_Barrel") != std::string::npos) {
            attachmentIndices[2] = 0;
        }
        else if (barrel.find("Muzzle_Break") != std::string::npos) {
            attachmentIndices[2] = 1;
        }
        else if (barrel.find("Compensator") != std::string::npos) {
            attachmentIndices[2] = 2;
        }
        else if (barrel.find("Flash_Hider") != std::string::npos) {
            attachmentIndices[2] = 3;
        }
    }

    // Process grip (index 1 in vector, position 2 in UI)
    if (!detectedAttachments[2].empty()) {
        std::string grip = detectedAttachments[2];

        if (grip.find("Vertical_Grip") != std::string::npos) {
            attachmentIndices[1] = 1;
        }
        else if (grip.find("Horizontal_Grip") != std::string::npos ||
            grip.find("Angled_Grip") != std::string::npos) {
            attachmentIndices[1] = 0;
        }
    }

    return attachmentIndices;
}

void DXGI::initializeRustDetector(cv::Mat& src) {
    for (const auto& weaponMask : rustMasks) {
        cv::Mat mask = cv::imdecode(weaponMask.mask, cv::IMREAD_GRAYSCALE);
        if (mask.empty()) {
            std::cerr << xorstr_("Failed to decode mask for: ") << weaponMask.name << std::endl;
            continue;
        }

        // Resize the mask to a standard size
        cv::resize(mask, mask, cv::Size(64, 64)); // Choose an appropriate size

        // Apply edge detection
        cv::Mat maskEdges;
        cv::Canny(mask, maskEdges, 30, 90);

        processedMasks.push_back({ weaponMask.name, maskEdges });
    }

    // Clear any existing weapon boxes
    weaponBoxes.clear();

    std::vector<BoxPercentage> weaponBoxPercentages = {
        {0.00f, 0.0f, 0.15f, 1.f},  // Adjust these percentages
        {0.17f, 0.0f, 0.15f, 1.f},  // to match the actual positions
        {0.34f, 0.0f, 0.15f, 1.f},  // of the weapon boxes in your UI
        {0.51f, 0.0f, 0.15f, 1.f},
        {0.679f, 0.0f, 0.15f, 1.f},
        {0.849f, 0.0f, 0.15f, 1.f}
    };

    for (const auto& box : weaponBoxPercentages) {
        int x = static_cast<int>(src.cols * box.x);
        int y = static_cast<int>(src.rows * box.y);
        int width = static_cast<int>(src.cols * box.width);
        int height = static_cast<int>(src.rows * box.height);
        weaponBoxes.emplace_back(x, y, width, height);
    }
}

// Helper function to create the mask
cv::Mat createMask(int size, int bottomRightWidth, int bottomRightHeight, int topLeftWidth, int topLeftHeight) {
    cv::Mat mask = cv::Mat::ones(size, size, CV_8UC1) * 255;
    mask(cv::Rect(size - bottomRightWidth, size - bottomRightHeight, bottomRightWidth, bottomRightHeight)).setTo(0);
    mask(cv::Rect(0, 0, topLeftWidth, topLeftHeight)).setTo(0);
    return mask;
}

std::string DXGI::detectWeaponTypeWithMask(const cv::Mat& weaponIcon) {
    static const int MASK_SIZE = 64;
    static const int bottomRightWidth = static_cast<int>(MASK_SIZE * 0.45f);
    static const int bottomRightHeight = static_cast<int>(MASK_SIZE * 0.27f);
    static const int topLeftWidth = static_cast<int>(MASK_SIZE * 0.2f);
    static const int topLeftHeight = static_cast<int>(MASK_SIZE * 0.4f);

    static const cv::Mat mask = createMask(MASK_SIZE, bottomRightWidth, bottomRightHeight, topLeftWidth, topLeftHeight);

    cv::Mat gray;
    cv::cvtColor(weaponIcon, gray, cv::COLOR_BGR2GRAY);
    cv::resize(gray, gray, cv::Size(64, 64));

    // Apply Gaussian blur to reduce noise
    //cv::GaussianBlur(gray, gray, cv::Size(3, 3), 0); 

    // Apply Canny edge detection with adjusted parameters
    cv::Mat edges;
    cv::Canny(gray, edges, 30, 90);

    // Apply the mask to the edge-detected image
    cv::Mat_<uchar> maskedEdges;
    edges.copyTo(maskedEdges, mask);

    std::string bestMatch;
    double bestMatchScore = 0.0;

    for (const auto& processedMask : processedMasks) {
        //cv::resize(mask, mask, weaponIcon.size());

        cv::Mat result;
        cv::matchTemplate(maskedEdges, processedMask.edges, result, cv::TM_CCORR_NORMED);
        double maxVal;
        cv::minMaxLoc(result, nullptr, &maxVal);

        if (maxVal > bestMatchScore) {
            bestMatchScore = maxVal;
            bestMatch = processedMask.name;
        }
    }

    return bestMatchScore > 0.2 ? bestMatch : xorstr_("Unknown Weapon");
}

void DXGI::detectWeaponRust(cv::Mat& src) {
    if (src.empty()) {
        return;
    }

    // Initialize variables properly
    int activeBoxIndex = -1;
    cv::Mat activeWeaponIcon;
    double maxColorScore = 0.0;

    const int targetHue = 102;
    const int targetSat = 212;
    const int targetVal = 144;

    // Get scores for all boxes first before drawing anything
    std::vector<double> colorScores(weaponBoxes.size(), 0.0);

    // First pass: calculate all color scores
    for (int i = 0; i < weaponBoxes.size(); ++i) {
        // Make sure the box is within bounds
        cv::Rect validBox = weaponBoxes[i] & cv::Rect(0, 0, src.cols, src.rows);
        if (validBox.width <= 0 || validBox.height <= 0) {
            continue;
        }

        cv::Mat boxROI = src(validBox);
        cv::Mat hsv;
        cv::cvtColor(boxROI, hsv, cv::COLOR_BGR2HSV);

        double colorScore = 0.0;
        double totalPixels = boxROI.total();
        int matchingPixels = 0;

        // Use a slightly more lenient threshold for better detection
        for (int y = 0; y < hsv.rows; ++y) {
            for (int x = 0; x < hsv.cols; ++x) {
                cv::Vec3b pixel = hsv.at<cv::Vec3b>(y, x);
                int hue = pixel[0];
                int sat = pixel[1];
                int val = pixel[2];

                // Calculate color similarity
                double hueDiff = std::min(std::abs(hue - targetHue), 180 - std::abs(hue - targetHue)) / 90.0;
                double satDiff = std::abs(sat - targetSat) / 255.0;
                double valDiff = std::abs(val - targetVal) / 255.0;

                double similarity = 1.0 - (hueDiff + satDiff + valDiff) / 3.0;

                // Lower the threshold from 0.8 to 0.7 for better detection
                if (similarity > 0.7) {
                    colorScore += similarity;
                    matchingPixels++;
                }
            }
        }

        // Normalize the color score
        colorScore /= totalPixels;
        double matchingRatio = matchingPixels / totalPixels;
        colorScore *= matchingRatio;

        // Store for logging
        colorScores[i] = colorScore;

        // Update if this is the highest color score so far
        if (colorScore > maxColorScore) {
            maxColorScore = colorScore;
            activeBoxIndex = i;
            activeWeaponIcon = boxROI.clone();
        }
    }

    // Second pass: draw rectangles after we've determined the active box
    //for (int i = 0; i < weaponBoxes.size(); ++i) {
    //    cv::Rect validBox = weaponBoxes[i] & cv::Rect(0, 0, src.cols, src.rows);
    //    if (validBox.width <= 0 || validBox.height <= 0) {
    //        continue;
    //    }

    //    // Log the scores
    //    std::cout << "Color score for box " << i << ": " << colorScores[i] << std::endl;

    //    // Draw green for active, red for inactive
    //    cv::Scalar boxColor = (i == activeBoxIndex) ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
    //    cv::rectangle(src, validBox, boxColor, 2); // Thicker line for visibility
    //}

    //std::cout << "Selected box: " << activeBoxIndex << std::endl;

    const double MIN_WEIGHTED_SCORE = 0.01;

    // Process the selected weapon
    if (activeBoxIndex != -1 && maxColorScore >= MIN_WEIGHTED_SCORE) {
        std::string detectedWeapon = detectWeaponTypeWithMask(activeWeaponIcon);

        settings.activeState.weaponOffOverride = false;

        if (detectedWeapon == xorstr_("Unknown Weapon")) {
            return;
        }

        // Find and set the weapon
        int weaponIndex = -1;
        for (size_t i = 0; i < settings.characters[settings.activeState.selectedCharacterIndex].weapondata.size(); ++i) {
            if (settings.characters[settings.activeState.selectedCharacterIndex].weapondata[i].weaponname == detectedWeapon) {
                weaponIndex = static_cast<int>(i);
                break;
            }
        }

        if (weaponIndex != -1) {
            settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] = weaponIndex;
            settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].rapidfire =
                settings.characters[settings.activeState.selectedCharacterIndex].weapondata[weaponIndex].rapidfire;
            settings.activeState.weaponDataChanged = true;
        }
        else {
            std::cout << xorstr_("Warning: Detected weapon not found in weapondata: ") << detectedWeapon << std::endl;
        }
    }
    else {
        settings.activeState.weaponOffOverride = true;
    }
}

void DXGI::overwatchDetector(cv::Mat& src) {
    static cv::Mat hsvImage, mask, downsampledSrc;
    static std::vector<std::vector<cv::Point>> contours;
    static cv::Mat dilateKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    static cv::Rect excludeROI;
    static cv::Size lastSize;
    static bool simdInitialized = false;
    static bool hasAVX512 = false;
    static bool hasAVX2 = false;
    static bool destroyed = true;

    // Temporal smoothing
    struct SmoothTarget {
        cv::Point2f pos = cv::Point2f(0, 0);
        float area = 0.f;
        int lastSeen = 0;
        float confidence = 0.f;
    };

    static std::vector<SmoothTarget> trackedTargets;
    static int frameCount = 0;
    const int maxTrackAge = settings.aimbotData.colourAimbotSettings.maxTrackAge;  // Maximum frames to keep tracking a disappeared target
    const float smoothingFactor = PERCENT(settings.aimbotData.colourAimbotSettings.trackSmoothingFactor);  // Higher = more smoothing, range 0-1

    frameCount++;

    if (!simdInitialized) {
        int avx = globals.startup.avx.load();
        hasAVX512 = avx == 2;
        hasAVX2 = avx >= 1;
        simdInitialized = true;
    }

    // Downsample with nearest neighbor to preserve thin lines
    cv::resize(src, downsampledSrc, cv::Size(src.cols / 2, src.rows / 2), 0, 0, cv::INTER_NEAREST);

    if (lastSize != downsampledSrc.size()) {
        lastSize = downsampledSrc.size();
        excludeROI = cv::Rect(
            static_cast<int>(downsampledSrc.cols * 0.385f),
            static_cast<int>(downsampledSrc.rows * 0.89f),
            static_cast<int>(downsampledSrc.cols * 0.23f),
            static_cast<int>(downsampledSrc.rows * 0.15f)
        );
        hsvImage.create(downsampledSrc.size(), CV_8UC3);
        mask.create(downsampledSrc.size(), CV_8UC1);
    }

    const int totalPixels = downsampledSrc.rows * downsampledSrc.cols;
    cv::cvtColor(downsampledSrc, hsvImage, cv::COLOR_BGR2HSV, 3);

    if (hasAVX512) {
        // Precompute comparison values and masks
        const __m512i h_lower = _mm512_set1_epi32(146);
        const __m512i h_upper = _mm512_set1_epi32(153);
        const __m512i s_thresh = _mm512_set1_epi32(130);
        const __m512i v_thresh = _mm512_set1_epi32(181);
        const __m512i gather_indices = _mm512_set_epi32(45, 42, 39, 36, 33, 30, 27, 24, 21, 18, 15, 12, 9, 6, 3, 0);
        const __m512i channel_mask = _mm512_set1_epi32(0xFF);

        // Pre-calculate ROI checks
        const int roi_start_x = excludeROI.x;
        const int roi_end_x = excludeROI.x + excludeROI.width;
        const int roi_start_y = excludeROI.y;
        const int roi_end_y = excludeROI.y + excludeROI.height;

        // Calculate number of full vectors per row
        const int vectors_per_row = downsampledSrc.cols / 16;
        const int remainder = downsampledSrc.cols % 16;

#pragma omp parallel for schedule(static)
        for (int y = 0; y < downsampledSrc.rows; y++) {
            const uint8_t* hsvRow = hsvImage.ptr<uint8_t>(y);
            uint8_t* maskRow = mask.ptr<uint8_t>(y);
            const bool in_roi_y = (y >= roi_start_y && y < roi_end_y);

            // Process full vectors
            for (int x = 0; x < vectors_per_row * 16; x += 16) {
                // Quick ROI check
                if (in_roi_y && x >= roi_start_x && x < roi_end_x) {
                    _mm_store_si128((__m128i*)(maskRow + x), _mm_setzero_si128());
                    continue;
                }

                // Load 48 bytes (16 pixels * 3 channels) using gather
                __m512i pixels = _mm512_i32gather_epi32(
                    gather_indices,
                    (const int*)(hsvRow + x * 3),
                    1
                );

                // Extract H, S, V channels using shifts and masks
                __m512i h = _mm512_and_si512(_mm512_srli_epi32(pixels, 0), channel_mask);
                __m512i s = _mm512_and_si512(_mm512_srli_epi32(pixels, 8), channel_mask);
                __m512i v = _mm512_and_si512(_mm512_srli_epi32(pixels, 16), channel_mask);

                // Perform comparisons - using compound operations
                __mmask16 final_mask =
                    _mm512_cmpge_epi32_mask(h, h_lower) &
                    _mm512_cmple_epi32_mask(h, h_upper) &
                    _mm512_cmpge_epi32_mask(s, s_thresh) &
                    _mm512_cmpge_epi32_mask(v, v_thresh);

                // Convert mask to bytes (0 or 255) and store
                _mm_store_si128(
                    (__m128i*)(maskRow + x),
                    _mm512_cvtepi32_epi8(_mm512_maskz_set1_epi32(final_mask, 255))
                );
            }

            // Handle remainder pixels using scalar operations
            if (remainder > 0) {
                const int start_x = vectors_per_row * 16;
                for (int x = start_x; x < downsampledSrc.cols; x++) {
                    if (in_roi_y && x >= roi_start_x && x < roi_end_x) {
                        maskRow[x] = 0;
                        continue;
                    }

                    const uint8_t* pixel = hsvRow + x * 3;
                    maskRow[x] = (pixel[0] >= 146 && pixel[0] <= 153 &&
                        pixel[1] >= 130 && pixel[2] >= 181) ? 255 : 0;
                }
            }
        }
    }
    else if (hasAVX2) {
#pragma omp parallel for
        for (int y = 0; y < downsampledSrc.rows; y++) {
            const uint8_t* hsvRow = hsvImage.ptr<uint8_t>(y);
            uint8_t* maskRow = mask.ptr<uint8_t>(y);

            for (int x = 0; x < downsampledSrc.cols; x += 32) {
                if (y >= excludeROI.y && y < excludeROI.y + excludeROI.height &&
                    x >= excludeROI.x && x < excludeROI.x + excludeROI.width) {
                    std::memset(maskRow + x, 0, 32);
                    continue;
                }

                alignas(32) uint8_t h_vals[32], s_vals[32], v_vals[32];
                alignas(32) uint8_t result[32];


                const uint8_t* pixel = hsvRow + x * 3;
                for (int j = 0; j < 32 && (x + j) < downsampledSrc.cols; j++) {
                    h_vals[j] = pixel[j * 3];
                    s_vals[j] = pixel[j * 3 + 1];
                    v_vals[j] = pixel[j * 3 + 2];
                }

                __m256i h = _mm256_load_si256((__m256i*)h_vals);
                __m256i s = _mm256_load_si256((__m256i*)s_vals);
                __m256i v = _mm256_load_si256((__m256i*)v_vals);

                // Create comparison masks
                __m256i h_min = _mm256_set1_epi8(static_cast<char>(146));
                __m256i h_max = _mm256_set1_epi8(static_cast<char>(153));
                __m256i s_min = _mm256_set1_epi8(static_cast<char>(130));
                __m256i v_min = _mm256_set1_epi8(static_cast<char>(181));

                // Correct comparisons for unsigned bytes
                __m256i h_mask = _mm256_and_si256(
                    _mm256_cmpeq_epi8(_mm256_max_epu8(h, h_min), h),
                    _mm256_cmpeq_epi8(_mm256_min_epu8(h, h_max), h)
                );
                __m256i s_mask = _mm256_cmpeq_epi8(_mm256_max_epu8(s, s_min), s);
                __m256i v_mask = _mm256_cmpeq_epi8(_mm256_max_epu8(v, v_min), v);

                // Combine masks and set result
                __m256i combined_mask = _mm256_and_si256(_mm256_and_si256(h_mask, s_mask), v_mask);

                // Convert mask to 0/255 values
                combined_mask = _mm256_and_si256(combined_mask, _mm256_set1_epi8(static_cast<char>(255)));

                _mm256_store_si256((__m256i*)result, combined_mask);
                std::memcpy(maskRow + x, result, 32);
            }
        }
    }
    else {
#pragma omp parallel for
        for (int y = 0; y < downsampledSrc.rows; y++) {
            const uint8_t* hsvRow = hsvImage.ptr<uint8_t>(y);
            uint8_t* maskRow = mask.ptr<uint8_t>(y);

            for (int x = 0; x < downsampledSrc.cols; x++) {
                if (y >= excludeROI.y && y < excludeROI.y + excludeROI.height &&
                    x >= excludeROI.x && x < excludeROI.x + excludeROI.width) {
                    maskRow[x] = 0;
                    continue;
                }

                const uint8_t* pixel = hsvRow + x * 3;
                maskRow[x] = (pixel[0] >= 146 && pixel[0] <= 153 &&
                    pixel[1] >= 130 && pixel[2] >= 181) ? 255 : 0;
            }
        }
    }

    cv::dilate(mask, mask, dilateKernel);

    // Use connected components instead of contours
    cv::Mat labels, stats, centroids;
    int numLabels = cv::connectedComponentsWithStats(mask, labels, stats, centroids);

    // Structure to hold component data for clustering
    struct Component {
        cv::Point center = cv::Point(0, 0);
        int area = 0;
        cv::Rect bounds = cv::Rect(0, 0, 0, 0);
        float density = 0.f;
    };
    std::vector<Component> validComponents;

    // First pass: collect valid components
    for (int label = 1; label < numLabels; label++) {
        int area = stats.at<int>(label, cv::CC_STAT_AREA);

        // Basic filtering for obviously invalid components
        if (area < settings.aimbotData.colourAimbotSettings.minArea || area >(src.rows * src.cols / 4)) {
            continue;
        }

        int width = stats.at<int>(label, cv::CC_STAT_WIDTH);
        int height = stats.at<int>(label, cv::CC_STAT_HEIGHT);
        int x = stats.at<int>(label, cv::CC_STAT_LEFT);
        int y = stats.at<int>(label, cv::CC_STAT_TOP);

        float density = static_cast<float>(area) / (width * height);
        if (density < PERCENT(settings.aimbotData.colourAimbotSettings.minDensity)) continue;  // Filter very sparse components

        Component comp;
        comp.center = cv::Point2f(centroids.at<double>(label, 0), centroids.at<double>(label, 1));
        comp.area = area;
        comp.bounds = cv::Rect(x, y, width, height);
        comp.density = density;

        validComponents.push_back(comp);
    }

    // Improved clustering with density-based merging
    const float minClusterDist = downsampledSrc.cols * PERCENT(settings.aimbotData.colourAimbotSettings.maxClusterDistance);  // Minimum distance between clusters
    std::vector<Component> clusters;
    std::vector<bool> used(validComponents.size(), false);

    for (size_t i = 0; i < validComponents.size(); i++) {
        if (used[i]) continue;

        Component cluster = validComponents[i];
        std::vector<size_t> clusterIndices = { i };
        used[i] = true;

        // Find components that belong to this cluster
        for (size_t j = i + 1; j < validComponents.size(); j++) {
            if (used[j]) continue;

            float dx = cluster.center.x - validComponents[j].center.x;
            float dy = cluster.center.y - validComponents[j].center.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            // Check if components are close enough and have similar density
            if (dist < minClusterDist &&
                std::abs(cluster.density - validComponents[j].density) < PERCENT(settings.aimbotData.colourAimbotSettings.maxClusterDensityDifferential)) {
                clusterIndices.push_back(j);
                used[j] = true;
            }
        }

        // If we found a valid cluster, compute its properties
        if (!clusterIndices.empty()) {
            Component finalCluster;
            finalCluster.center = cv::Point2f(0, 0);
            finalCluster.area = 0;
            cv::Rect bounds = validComponents[clusterIndices[0]].bounds;

            float totalWeight = 0;
            for (size_t idx : clusterIndices) {
                float weight = validComponents[idx].area * validComponents[idx].density;
                finalCluster.center += validComponents[idx].center * weight;
                finalCluster.area += validComponents[idx].area;
                bounds |= validComponents[idx].bounds;
                totalWeight += weight;
            }

            if (totalWeight > 0) {
                finalCluster.center *= 1.0f / totalWeight;
                finalCluster.bounds = bounds;
                finalCluster.density = finalCluster.area / (bounds.width * bounds.height);
                clusters.push_back(finalCluster);
            }
        }
    }

    // Update tracked targets
    std::vector<bool> clusterMatched(clusters.size(), false);
    std::vector<SmoothTarget> newTrackedTargets;

    // Update existing tracks
    for (const auto& track : trackedTargets) {
        bool matched = false;
        float bestDist = downsampledSrc.cols * PERCENT(settings.aimbotData.colourAimbotSettings.maxClusterDistance);  // Maximum tracking distance
        int bestIdx = -1;

        // Find the closest unmatched cluster
        for (size_t i = 0; i < clusters.size(); i++) {
            if (clusterMatched[i]) continue;

            float dx = track.pos.x - clusters[i].center.x;
            float dy = track.pos.y - clusters[i].center.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < bestDist) {
                bestDist = dist;
                bestIdx = i;
                matched = true;
            }
        }

        if (matched) {
            clusterMatched[bestIdx] = true;
            SmoothTarget newTrack = track;
            // Apply exponential smoothing
            newTrack.pos = track.pos * smoothingFactor +
                cv::Point2f(clusters[bestIdx].center) * (1.0f - smoothingFactor);
            newTrack.area = track.area * smoothingFactor +
                clusters[bestIdx].area * (1.0f - smoothingFactor);
            newTrack.lastSeen = frameCount;
            newTrack.confidence = std::min(1.0f, track.confidence + PERCENT(settings.aimbotData.colourAimbotSettings.trackConfidenceRate));
            newTrackedTargets.push_back(newTrack);
        }
        else if (frameCount - track.lastSeen < maxTrackAge) {
            // Keep track alive but reduce confidence
            SmoothTarget newTrack = track;
            newTrack.confidence = std::max(0.0f, track.confidence - PERCENT(settings.aimbotData.colourAimbotSettings.trackConfidenceRate));
            newTrackedTargets.push_back(newTrack);
        }
    }

    // Add new tracks for unmatched clusters
    for (size_t i = 0; i < clusters.size(); i++) {
        if (!clusterMatched[i]) {
            SmoothTarget newTrack;
            newTrack.pos = clusters[i].center;
            newTrack.area = static_cast<float>(clusters[i].area);
            newTrack.lastSeen = frameCount;
            newTrack.confidence = 0.3f;  // Initial confidence
            newTrackedTargets.push_back(newTrack);
        }
    }

    trackedTargets = newTrackedTargets;

    const cv::Point imageCenter(downsampledSrc.cols / 2, downsampledSrc.rows / 2);
    float minDistance = std::numeric_limits<float>::max();
    cv::Point targetPoint;
    bool targetFound = false;

    for (const auto& track : trackedTargets) {
        if (track.confidence < 0.5f) continue;  // Skip low confidence tracks

        float dx = track.pos.x - imageCenter.x;
        float dy = track.pos.y - imageCenter.y;
        float distance = std::abs(dx) + std::abs(dy);

        if (distance < minDistance) {
            minDistance = distance;
            targetPoint = cv::Point2f(track.pos.x, track.pos.y - settings.aimbotData.colourAimbotSettings.aimHeight);
            targetFound = true;
        }

        if (settings.aimbotData.colourAimbotSettings.debugView) {
            cv::Scalar color(0, 255 * track.confidence, 255 * (1.0f - track.confidence));
            int radius = std::max(3, static_cast<int>(std::sqrt(track.area) * 0.5f));
            cv::circle(downsampledSrc, track.pos, radius, color, 1);
            cv::circle(downsampledSrc, track.pos, 2, color, -1);
        }
    }

    if (targetFound) {
        std::vector<float> sens = settings.activeState.sensMultiplier_SensOnly;

        static const float fovScale = constants.OW360DIST * 2.0f / 360.0f;
        const float pixelsPerDegree = static_cast<float>(src.cols) / settings.globalSettings.fov;  // Note: using original size

        // Scale back up to original coordinates
        const float scaledX = static_cast<float>((targetPoint.x * 2) - src.cols / 2);
        const float scaledY = static_cast<float>((targetPoint.y * 2) - src.rows / 2);

        settings.aimbotData.correctionX = static_cast<int>((scaledX / pixelsPerDegree) * fovScale * sens[0]);
        settings.aimbotData.correctionY = static_cast<int>((scaledY / pixelsPerDegree) * fovScale * sens[1]);

        if (settings.aimbotData.colourAimbotSettings.debugView) {
            cv::circle(downsampledSrc, targetPoint, 3, cv::Scalar(255, 0, 0), -1);
        }
    }
    else {
        settings.aimbotData.correctionX = 0;
        settings.aimbotData.correctionY = 0;
    }

    if (settings.aimbotData.colourAimbotSettings.debugView) {
        cv::imshow(xorstr_("Source"), downsampledSrc);
        //cv::imshow("Mask", mask);
        cv::waitKey(1);
        destroyed = false;
    } else if (!destroyed && !settings.aimbotData.colourAimbotSettings.debugView) {
		cv::destroyWindow(xorstr_("Source"));
		//cv::destroyWindow("Mask");
		destroyed = true;
	}
}

int DXGI::isOperatorScreenR6(cv::Mat& src) {
    if (src.empty()) {
		return 0;
	}

    // Define ROIs for primary and secondary weapon selection areas
    float primaryRoiXPercent = 0.f;
    float primaryRoiYPercent = 0.f;
    float primaryRoiWidthPercent = 1.f;
    float primaryRoiHeightPercent = 0.15f;

    float secondaryRoiXPercent = 0.f;
    float secondaryRoiYPercent = 0.85f;
    float secondaryRoiWidthPercent = 1.f;
    float secondaryRoiHeightPercent = 0.15f;

    // Calculate actual ROI coordinates for primary weapon
    cv::Rect primaryRoi(
        static_cast<int>(src.cols * primaryRoiXPercent),
        static_cast<int>(src.rows * primaryRoiYPercent),
        static_cast<int>(src.cols * primaryRoiWidthPercent),
        static_cast<int>(src.rows * primaryRoiHeightPercent)
    );

    // Calculate actual ROI coordinates for secondary weapon
    cv::Rect secondaryRoi(
        static_cast<int>(src.cols * secondaryRoiXPercent),
        static_cast<int>(src.rows * secondaryRoiYPercent),
        static_cast<int>(src.cols * secondaryRoiWidthPercent),
        static_cast<int>(src.rows * secondaryRoiHeightPercent)
    );

    // Ensure the ROIs are within the source image bounds
    primaryRoi &= cv::Rect(0, 0, src.cols, src.rows);
    secondaryRoi &= cv::Rect(0, 0, src.cols, src.rows);

    // Skip if either ROI is invalid
    if (primaryRoi.width == 0 || primaryRoi.height == 0 ||
        secondaryRoi.width == 0 || secondaryRoi.height == 0) {
        return 0;
    }

    // Extract the ROIs
    cv::Mat primaryRegion = src(primaryRoi);
    cv::Mat secondaryRegion = src(secondaryRoi);

    // Convert to HSV for better color detection
    cv::Mat primaryHsv, secondaryHsv;
    cv::cvtColor(primaryRegion, primaryHsv, cv::COLOR_BGR2HSV);
    cv::cvtColor(secondaryRegion, secondaryHsv, cv::COLOR_BGR2HSV);

    // Count blue pixels in each ROI
    int primaryBlueCount = 0;
    int secondaryBlueCount = 0;

    // Check primary ROI
    for (int y = 0; y < primaryHsv.rows; y++) {
        for (int x = 0; x < primaryHsv.cols; x++) {
            cv::Vec3b pixel = primaryHsv.at<cv::Vec3b>(y, x);
            // Blue detection - adjust these thresholds as needed
            if (pixel[0] > 95 && pixel[0] < 115 && pixel[1] > 150 && pixel[2] > 150) {
                primaryBlueCount++;
            }
        }
    }

    // Check secondary ROI
    for (int y = 0; y < secondaryHsv.rows; y++) {
        for (int x = 0; x < secondaryHsv.cols; x++) {
            cv::Vec3b pixel = secondaryHsv.at<cv::Vec3b>(y, x);
            // Blue detection - same criteria as primary
            if (pixel[0] > 95 && pixel[0] < 115 && pixel[1] > 150 && pixel[2] > 150) {
                secondaryBlueCount++;
            }
        }
    }

    // Calculate the percentage of blue pixels in each ROI
    float primaryBluePercent = static_cast<float>(primaryBlueCount) / primaryHsv.total();
    float secondaryBluePercent = static_cast<float>(secondaryBlueCount) / secondaryHsv.total();

    // Determine which ROI has the most blue (if any)
    const float blueThreshold = 0.1f; // 10% of pixels need to be blue

    if (primaryBluePercent > blueThreshold && primaryBluePercent > secondaryBluePercent) {
        return 1; // Primary weapon selected
    }
    else if (secondaryBluePercent > blueThreshold && secondaryBluePercent > primaryBluePercent) {
        return 2; // Secondary weapon selected
    }
    else {
        return 0; // Neither selected
    }
}

float DXGI::getOperatorDetectionConfidence(cv::Mat& roi) {
    // First, normalize and hash the icon
    cv::Mat normalizedRoi = normalizeIconSize(roi, 64, 64);
    IconHash roiHash = hashIcon(normalizedRoi, 4);

    // Find best match among operator hashes
    int minHammingDistance = HASH_SIZE;
    float bestMatchPercentage = 100.0f;
    std::string detectedOperator;

    for (const auto& pair : operatorHashes) {
        int distance = utils.hammingDistance(roiHash, pair.first);
        float percentDiff = static_cast<float>(distance) / HASH_SIZE * 100.0f;

        if (distance < minHammingDistance) {
            minHammingDistance = distance;
            bestMatchPercentage = percentDiff;
            detectedOperator = pair.second;
        }
    }

    // Convert the percentage difference to a confidence score (0-1)
    // Lower percentage difference means higher confidence
    float confidence = 1.0f - (bestMatchPercentage / 100.0f);

    return confidence;
}

std::vector<float> DXGI::getAttachmentDetectionConfidence(cv::Mat& attachmentRegion) {
    std::vector<float> confidenceScores(4, 0.0f); // 4 attachment slots

    if (attachmentRegion.empty()) {
        return confidenceScores;
    }

    // Define ROIs for different attachment slots
    std::vector<cv::Rect> attachmentRois;
    float iconHeight = 0.25f;
    float iconWidth = 1.f;
    float iconX = 0.f;

    // Define ROIs for each attachment type
    float scopeY = 0.f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(scopeY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    float barrelY = 0.25f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(barrelY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    float gripY = 0.50f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(gripY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    float laserY = 0.75f;
    attachmentRois.push_back(cv::Rect(
        static_cast<int>(iconX * attachmentRegion.cols),
        static_cast<int>(laserY * attachmentRegion.rows),
        static_cast<int>(iconWidth * attachmentRegion.cols),
        static_cast<int>(iconHeight * attachmentRegion.rows)
    ));

    // Process each attachment region
    for (int i = 0; i < attachmentRois.size(); i++) {
        cv::Mat attachment = attachmentRegion(attachmentRois[i]);
        cv::Mat normalizedAttachment = normalizeIconSize(attachment, 32, 32);
        IconHash attachmentHash = hashIcon(normalizedAttachment, 2);

        // Find best match and calculate confidence
        int minHammingDistance = HASH_SIZE;
        float percentDiff = 100.0f;

        for (const auto& pair : attachmentHashes) {
            int distance = utils.hammingDistance(attachmentHash, pair.first);

            if (distance < minHammingDistance) {
                minHammingDistance = distance;
                percentDiff = static_cast<float>(minHammingDistance) / HASH_SIZE * 100.0f;
            }
        }

        // Convert percentage difference to confidence score (0-1)
        confidenceScores[i] = 1.0f - (percentDiff / 100.0f);
    }

    return confidenceScores;
}

DXGI::ROIParameters DXGI::optimizeOperatorDetectionROI(cv::Mat& src, float initialX, float initialY, float initialWidth, float initialHeight, float stepSize) {
    // Initial parameters
    ROIParameters bestParams;
    bestParams.ratioX = initialX;
    bestParams.ratioY = initialY;
    bestParams.ratioWidth = initialWidth;
    bestParams.ratioHeight = initialHeight;
    bestParams.confidence = 0.0f;

    // Desktop dimensions
    int desktopWidth = src.cols;
    int desktopHeight = src.rows;

    // Number of steps to try in each direction
    const int numSteps = 10;

    // For logging progress
    int totalIterations = (2 * numSteps + 1) * (2 * numSteps + 1) * (2 * numSteps + 1) * (2 * numSteps + 1);
    int currentIteration = 0;

    std::cout << "Starting ROI optimization (this may take a while)..." << std::endl;

    // Temp variables to avoid unnecessary conversions in the loops
    cv::Mat roiImage;
    int x, y, width, height;
    bool result;
    float confidence;

    // Grid search over all parameters
    for (int xStep = -numSteps; xStep <= numSteps; xStep++) {
        float testX = initialX + xStep * stepSize;

        for (int yStep = -numSteps; yStep <= numSteps; yStep++) {
            float testY = initialY + yStep * stepSize;

            for (int wStep = -numSteps; wStep <= numSteps; wStep++) {
                float testWidth = initialWidth + wStep * stepSize;

                for (int hStep = -numSteps; hStep <= numSteps; hStep++) {
                    float testHeight = initialHeight + hStep * stepSize;

                    // Ensure valid parameter ranges
                    if (testX < 0.0f || testY < 0.0f || testWidth <= 0.0f || testHeight <= 0.0f ||
                        testX + testWidth > 1.0f || testY + testHeight > 1.0f) {
                        continue;
                    }

                    // Calculate ROI dimensions
                    x = static_cast<int>(testX * desktopWidth);
                    y = static_cast<int>(testY * desktopHeight);
                    width = static_cast<int>(testWidth * desktopWidth);
                    height = static_cast<int>(testHeight * desktopHeight);

                    // Ensure the ROI is within bounds
                    x = std::max(0, x);
                    y = std::max(0, y);
                    width = std::min(width, desktopWidth - x);
                    height = std::min(height, desktopHeight - y);

                    if (width <= 0 || height <= 0) continue;

                    // Extract ROI and detect operator
                    cv::Rect roi(x, y, width, height);
                    roiImage = src(roi);

                    // This part needs to be modified to return a confidence value instead of just boolean
                    // For now, let's assume we have a version of detectOperatorR6 that returns a confidence score
                    confidence = getOperatorDetectionConfidence(roiImage);

                    // Update progress
                    currentIteration++;
                    if (currentIteration % 100 == 0) {
                        std::cout << "Progress: " << (currentIteration * 100.0f / totalIterations) << "%" << std::endl;
                    }

                    // Update best parameters if this ROI is better
                    if (confidence > bestParams.confidence) {
                        bestParams.ratioX = testX;
                        bestParams.ratioY = testY;
                        bestParams.ratioWidth = testWidth;
                        bestParams.ratioHeight = testHeight;
                        bestParams.confidence = confidence;
                    }
                }
            }
        }
    }

    // Print the optimal parameters
    std::cout << "operatorDetectionRatioX = " << bestParams.ratioX << "f;" << std::endl;
    std::cout << "operatorDetectionRatioY = " << bestParams.ratioY << "f;" << std::endl;
    std::cout << "operatorDetectionRatioWidth = " << bestParams.ratioWidth << "f;" << std::endl;
    std::cout << "operatorDetectionRatioHeight = " << bestParams.ratioHeight << "f;" << std::endl;
    std::cout << "Confidence: " << bestParams.confidence << std::endl;

    return bestParams;
}

DXGI::ROIParameters DXGI::optimizeAttachmentDetectionROI(cv::Mat& src, float initialX, float initialY, float initialWidth, float initialHeight, float stepSize) {
    // Initial parameters
    ROIParameters bestParams;
    bestParams.ratioX = initialX;
    bestParams.ratioY = initialY;
    bestParams.ratioWidth = initialWidth;
    bestParams.ratioHeight = initialHeight;
    bestParams.confidence = 0.0f;

    // Desktop dimensions
    int desktopWidth = src.cols;
    int desktopHeight = src.rows;

    // Number of steps to try in each direction
    const int numSteps = 10;

    // For logging progress
    int totalIterations = (2 * numSteps + 1) * (2 * numSteps + 1) * (2 * numSteps + 1) * (2 * numSteps + 1);
    int currentIteration = 0;

    std::cout << "Starting Attachment ROI optimization (this may take a while)..." << std::endl;

    // Grid search over all parameters
    for (int xStep = -numSteps; xStep <= numSteps; xStep++) {
        float testX = initialX + xStep * stepSize;

        for (int yStep = -numSteps; yStep <= numSteps; yStep++) {
            float testY = initialY + yStep * stepSize;

            for (int wStep = -numSteps; wStep <= numSteps; wStep++) {
                float testWidth = initialWidth + wStep * stepSize;

                for (int hStep = -numSteps; hStep <= numSteps; hStep++) {
                    float testHeight = initialHeight + hStep * stepSize;

                    // Ensure valid parameter ranges
                    if (testX < 0.0f || testY < 0.0f || testWidth <= 0.0f || testHeight <= 0.0f ||
                        testX + testWidth > 1.0f || testY + testHeight > 1.0f) {
                        continue;
                    }

                    // Calculate ROI dimensions
                    int x = static_cast<int>(testX * desktopWidth);
                    int y = static_cast<int>(testY * desktopHeight);
                    int width = static_cast<int>(testWidth * desktopWidth);
                    int height = static_cast<int>(testHeight * desktopHeight);

                    // Ensure the ROI is within bounds
                    x = std::max(0, x);
                    y = std::max(0, y);
                    width = std::min(width, desktopWidth - x);
                    height = std::min(height, desktopHeight - y);

                    if (width <= 0 || height <= 0) continue;

                    // Extract ROI and detect attachments
                    cv::Rect roi(x, y, width, height);
                    cv::Mat roiImage = src(roi);

                    // Get confidence scores for all attachment slots
                    std::vector<float> confidenceScores = getAttachmentDetectionConfidence(roiImage);

                    // Focus on first and last slots (scope and laser)
                    float firstSlotConfidence = confidenceScores[0];
                    float lastSlotConfidence = confidenceScores[3];

                    // Calculate overall confidence score - prioritize minimum confidence
                    // This ensures both slots have good detection
                    float overallConfidence = std::min(firstSlotConfidence, lastSlotConfidence);

                    // Update progress
                    currentIteration++;
                    if (currentIteration % 100 == 0) {
                        std::cout << "Progress: " << (currentIteration * 100.0f / totalIterations) << "%, "
                            << "Best confidence so far: " << bestParams.confidence << std::endl;
                    }

                    // Update best parameters if this ROI is better
                    if (overallConfidence > bestParams.confidence) {
                        bestParams.ratioX = testX;
                        bestParams.ratioY = testY;
                        bestParams.ratioWidth = testWidth;
                        bestParams.ratioHeight = testHeight;
                        bestParams.confidence = overallConfidence;

                        // Log improvements
                        std::cout << "New best parameters found! Confidence: " << overallConfidence
                            << " (First: " << firstSlotConfidence
                            << ", Last: " << lastSlotConfidence << ")" << std::endl;
                    }
                }
            }
        }
    }

    // Print the optimal parameters
    std::cout << "attachRegionRatioX = " << bestParams.ratioX << "f;" << std::endl;
    std::cout << "attachRegionRatioY = " << bestParams.ratioY << "f;" << std::endl;
    std::cout << "attachRegionRatioWidth = " << bestParams.ratioWidth << "f;" << std::endl;
    std::cout << "attachRegionRatioHeight = " << bestParams.ratioHeight << "f;" << std::endl;
    std::cout << "Final Confidence: " << bestParams.confidence << std::endl;

    return bestParams;
}

void DXGI::runOperatorOptimiser(float x, float y, float width, float height, cv::Mat& src) {
    ROIParameters optimizedParams = optimizeOperatorDetectionROI(
        src,
        x, y, width, height,
        0.001f
    );

    // Save the optimized parameters
    x = optimizedParams.ratioX;
    y = optimizedParams.ratioY;
    width = optimizedParams.ratioWidth;
    height = optimizedParams.ratioHeight;

    optimizedParams = optimizeOperatorDetectionROI(
        src,
        x, y, width, height,
        0.0001f
    );

    // Pause until the user presses a key to continue
    std::cout << "Press any key to continue..." << std::endl;
    std::cin.ignore();
}

void DXGI::runAttachmentOptimiser(float x, float y, float width, float height, cv::Mat& src) {
    ROIParameters optimizedParams = optimizeAttachmentDetectionROI(
        src,
        x, y, width, height,
        0.001f
    );

    // Save the optimized parameters
    x = optimizedParams.ratioX;
    y = optimizedParams.ratioY;
    width = optimizedParams.ratioWidth;
    height = optimizedParams.ratioHeight;

    std::cout << "attachRegionRatioX = " << x << "f;" << std::endl;
	std::cout << "attachRegionRatioY = " << y << "f;" << std::endl;
	std::cout << "attachRegionRatioWidth = " << width << "f;" << std::endl;
	std::cout << "attachRegionRatioHeight = " << height << "f;" << std::endl;

    optimizedParams = optimizeAttachmentDetectionROI(
        src,
        x, y, width, height,
        0.0001f
    );

    // Pause until the user presses a key to continue
    std::cout << "Press any key to continue..." << std::endl;
    std::cin.ignore();
}

void DXGI::detectAttachmentsR6(cv::Mat& src) {
    if (src.empty()) {
        return;
    }

    //////////////////////////////////////////////////////
    ///////// CHECK SELECTED WEAPON
    //////////////////////////////////////////////////////

    // Define ratios for crop region
    float screenCheckRatioX = 0.f;
    float screenCheckRatioY = 0.f;
    float screenCheckRatioWidth = 0.f; // 0.03f for testing
    float screenCheckRatioHeight = 0.f;

    switch (settings.globalSettings.aspect_ratio) {
    case 0:
        screenCheckRatioX = 0.021f;
        screenCheckRatioY = 0.41f;
        screenCheckRatioWidth = 0.0075f;
        screenCheckRatioHeight = 0.199f;
        break;
    case 1:
        screenCheckRatioX = 0.021f;
        screenCheckRatioY = 0.432f;
        screenCheckRatioWidth = 0.0075f;
		screenCheckRatioHeight = 0.15f;
        break;
    case 2:
        screenCheckRatioX = 0.021f;
        screenCheckRatioY = 0.436f;
        screenCheckRatioWidth = 0.0075f;
        screenCheckRatioHeight = 0.142f;
        break;
    case 3:
        screenCheckRatioX = 0.021f;
        screenCheckRatioY = 0.423f;
        screenCheckRatioWidth = 0.0075f;
        screenCheckRatioHeight = 0.17f;
        break;
    case 4:
        screenCheckRatioX = 0.021f;
        screenCheckRatioY = 0.419f;
        screenCheckRatioWidth = 0.0075f;
        screenCheckRatioHeight = 0.18f;
        break;
    case 5:
        screenCheckRatioX = 0.021f;
        screenCheckRatioY = 0.414f;
        screenCheckRatioWidth = 0.0075f;
        screenCheckRatioHeight = 0.188f;
        break;
    case 6:
        screenCheckRatioX = 0.052f;
        screenCheckRatioY = 0.41f;
        screenCheckRatioWidth = 0.0065f;
        screenCheckRatioHeight = 0.199f;
        break;
    case 7:
        screenCheckRatioX = 0.14f;
        screenCheckRatioY = 0.41f;
        screenCheckRatioWidth = 0.006f;
        screenCheckRatioHeight = 0.199f;
        break;
    }

    // Calculate the region of interest (ROI) based on ratios
    int desktopWidth = globals.capture.desktopWidth.load();
    int desktopHeight = globals.capture.desktopHeight.load();

    int screenCheckX = static_cast<int>(screenCheckRatioX * desktopWidth);
    int screenCheckY = static_cast<int>(screenCheckRatioY * desktopHeight);
    int screenCheckWidth = static_cast<int>(screenCheckRatioWidth * desktopWidth);
    int screenCheckHeight = static_cast<int>(screenCheckRatioHeight * desktopHeight);

    // Ensure the ROI is within the bounds of the desktopMat
    screenCheckX = std::max(0, screenCheckX);
    screenCheckY = std::max(0, screenCheckY);
    screenCheckWidth = std::min(screenCheckWidth, desktopWidth - screenCheckX);
    screenCheckHeight = std::min(screenCheckHeight, desktopHeight - screenCheckY);

    cv::Rect screenCheckRoi(screenCheckX, screenCheckY, screenCheckWidth, screenCheckHeight);

    cv::Mat screenCheckRegion = src(screenCheckRoi);

    int selectedWeapon = isOperatorScreenR6(screenCheckRegion);
	if (selectedWeapon == 0) {
		std::cout << "Not operator screen or no weapon selected" << std::endl;
		return;
	}

    //////////////////////////////////////////////////////
    ///////// OPERATOR DETECTION REGION
    //////////////////////////////////////////////////////

    static bool detectedOperator = false;
    static bool useShootingRangeOffset = false;

    float operatorDetectionRatioX = 0.f;
    float operatorDetectionRatioY = 0.f;
    float operatorDetectionRatioWidth = 0.f;
    float operatorDetectionRatioHeight = 0.f;

    // Define ratios for operatorDetection region
    switch (settings.globalSettings.aspect_ratio) {
    case 0:
        operatorDetectionRatioX = 0.3934f * (useShootingRangeOffset ? 1.138f : 1.f);
        operatorDetectionRatioY = 0.f;
        operatorDetectionRatioWidth = 0.0243f;
        operatorDetectionRatioHeight = 0.0514f;
        break;
    case 1:
        operatorDetectionRatioX = 0.3934f * (useShootingRangeOffset ? 1.138f : 1.f);
        operatorDetectionRatioY = 0.1244f;
        operatorDetectionRatioWidth = 0.0243f;
        operatorDetectionRatioHeight = 0.0396f;
        //runOptimiser(operatorDetectionRatioX, operatorDetectionRatioY, operatorDetectionRatioWidth, operatorDetectionRatioHeight, src);
        break;
    case 2:
        operatorDetectionRatioX = 0.3934f * (useShootingRangeOffset ? 1.138f : 1.f);
        operatorDetectionRatioY = 0.1487f;
        operatorDetectionRatioWidth = 0.0243f;
        operatorDetectionRatioHeight = 0.036f;
        break;
    case 3:
        operatorDetectionRatioX = 0.3934f * (useShootingRangeOffset ? 1.138f : 1.f);
        operatorDetectionRatioY = 0.0785f;
        operatorDetectionRatioWidth = 0.0243f;
        operatorDetectionRatioHeight = 0.0417f;
        break;
    case 4:
        operatorDetectionRatioX = 0.3934f * (useShootingRangeOffset ? 1.138f : 1.f);
        operatorDetectionRatioY = 0.0494f;
        operatorDetectionRatioWidth = 0.0243f;
        operatorDetectionRatioHeight = 0.0473f;
        break;
    case 5:
        operatorDetectionRatioX = 0.3934f * (useShootingRangeOffset ? 1.138f : 1.f);
        operatorDetectionRatioY = 0.0306f;
        operatorDetectionRatioWidth = 0.0239f;
        operatorDetectionRatioHeight = 0.048f;
        break;
    case 6:
        operatorDetectionRatioX = 0.4f * (useShootingRangeOffset ? 1.127f : 1.f);
        operatorDetectionRatioY = 0.f;
        operatorDetectionRatioWidth = 0.0231f;
        operatorDetectionRatioHeight = 0.0514f;
        break;
    case 7:
        operatorDetectionRatioX = 0.4196f * (useShootingRangeOffset ? 1.097f : 1.f);
        operatorDetectionRatioY = 0.f;
        operatorDetectionRatioWidth = 0.0184f;
        operatorDetectionRatioHeight = 0.0514f;
        break;
    }

    // Calculate the region of interest (ROI) based on ratios
    int operatorDetectionX = static_cast<int>(operatorDetectionRatioX * desktopWidth);
    int operatorDetectionY = static_cast<int>(operatorDetectionRatioY * desktopHeight);
    int operatorDetectionWidth = static_cast<int>(operatorDetectionRatioWidth * desktopWidth);
    int operatorDetectionHeight = static_cast<int>(operatorDetectionRatioHeight * desktopHeight);

    // Ensure the ROI is within the bounds of the desktopMat
    operatorDetectionX = std::max(0, operatorDetectionX);
    operatorDetectionY = std::max(0, operatorDetectionY);
    operatorDetectionWidth = std::min(operatorDetectionWidth, desktopWidth - operatorDetectionX);
    operatorDetectionHeight = std::min(operatorDetectionHeight, desktopHeight - operatorDetectionY);

    cv::Rect operatorDetectionRoi(operatorDetectionX, operatorDetectionY, operatorDetectionWidth, operatorDetectionHeight);

    cv::Mat operatorDetectionRegion = src(operatorDetectionRoi);

    detectedOperator = detectOperatorR6(operatorDetectionRegion);

    if (!detectedOperator) {
        useShootingRangeOffset = !useShootingRangeOffset;
        std::cout << "Couldn't detect operator" << std::endl;
		return;
    }

    //////////////////////////////////////////////////////
    ///////// ATTACHMENT REGION
    //////////////////////////////////////////////////////

    // Define ratios for crop region
    float attachRegionRatioX = 0.f;
    float attachRegionRatioY = 0.f;
    float attachRegionRatioWidth = 0.f;
    float attachRegionRatioHeight = 0.f;

    // Set aspect ratio-specific values
    switch (settings.globalSettings.aspect_ratio) {
    case 0:
        attachRegionRatioX = 0.3426f;
        attachRegionRatioY = 0.3056f;
        attachRegionRatioWidth = 0.0227f;
        attachRegionRatioHeight = 0.1195f;
        break;
    case 1:
        attachRegionRatioX = 0.3426f;
        attachRegionRatioY = 0.3549f;
        attachRegionRatioWidth = 0.0227f;
        attachRegionRatioHeight = 0.0889f;
        //runAttachmentOptimiser(attachRegionRatioX, attachRegionRatioY, attachRegionRatioWidth, attachRegionRatioHeight, src);
        break;
    case 2:
        attachRegionRatioX = 0.3426f;
        attachRegionRatioY = 0.3639f;
        attachRegionRatioWidth = 0.0227f;
        attachRegionRatioHeight = 0.0834f;
        break;
    case 3:
        attachRegionRatioX = 0.3426f;
        attachRegionRatioY = 0.3375f;
        attachRegionRatioWidth = 0.0227f;
        attachRegionRatioHeight = 0.0994f;
        break;
    case 4:
        attachRegionRatioX = 0.3426f;
        attachRegionRatioY = 0.3264f;
        attachRegionRatioWidth = 0.0227f;
        attachRegionRatioHeight = 0.107f;
        break;
    case 5:
        attachRegionRatioX = 0.343f;
        attachRegionRatioY = 0.3174f;
        attachRegionRatioWidth = 0.0219f;
        attachRegionRatioHeight = 0.1125f;
        break;
    case 6:
        attachRegionRatioX = 0.3528f;
        attachRegionRatioY = 0.3056f;
        attachRegionRatioWidth = 0.0208f;
        attachRegionRatioHeight = 0.1195f;
        break;
    case 7:
        attachRegionRatioX = 0.3817f;
        attachRegionRatioY = 0.3056f;
        attachRegionRatioWidth = 0.0168f;
        attachRegionRatioHeight = 0.1195f;
        break;
    }

    // Calculate attachment Region ROI
    int attachRegionX = static_cast<int>(attachRegionRatioX * desktopWidth);
    int attachRegionY = static_cast<int>(attachRegionRatioY * desktopHeight);
    int attachRegionWidth = static_cast<int>(attachRegionRatioWidth * desktopWidth);
    int attachRegionHeight = static_cast<int>(attachRegionRatioHeight * desktopHeight);

    // Ensure bounds
    attachRegionX = std::max(0, attachRegionX);
    attachRegionY = std::max(0, attachRegionY);
    attachRegionWidth = std::min(attachRegionWidth, desktopWidth - attachRegionX);
    attachRegionHeight = std::min(attachRegionHeight, desktopHeight - attachRegionY);

    cv::Rect attachRegionRoi(attachRegionX, attachRegionY, attachRegionWidth, attachRegionHeight);
    cv::Mat attachmentRegion = src(attachRegionRoi);

    // Call the new function to detect attachments
    std::vector<int> attachmentIndices = detectAttachmentsR6FromRegion(attachmentRegion);

    // Update the weapon attachments if we found any
    if (settings.activeState.selectedCharacterIndex < settings.characters.size()) {
        int weaponIndex = selectedWeapon == 1 ?
            settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] :
            settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[1];

        if (weaponIndex < settings.characters[settings.activeState.selectedCharacterIndex].weapondata.size()) {
            auto& weapon = settings.characters[settings.activeState.selectedCharacterIndex].weapondata[weaponIndex];

            // Ensure we have enough space for attachments
            if (weapon.attachments.size() < 3) {
                weapon.attachments.resize(3, 0);
            }

            // Copy detected attachments to weapon
            for (int i = 0; i < 3 && i < attachmentIndices.size(); i++) {
                weapon.attachments[i] = attachmentIndices[i];
            }

            settings.activeState.weaponDataChanged = true;
            globals.filesystem.unsavedChanges.store(true);
        }
    }

    // Debug visualization
	/*cv::imshow("Attachment Region", attachmentRegion);
    cv::waitKey(1);*/
}