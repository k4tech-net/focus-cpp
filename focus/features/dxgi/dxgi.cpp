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
    gDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));

    // Get DXGI adapter
    IDXGIAdapter* dxgiAdapter = nullptr;
    dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter));

    // Get output
    IDXGIOutput* dxgiOutput = nullptr;
    dxgiAdapter->EnumOutputs(0, &dxgiOutput); // Assuming first output

    // Get output 1
    IDXGIOutput1* dxgiOutput1 = nullptr;
    dxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgiOutput1));

    // Create output duplication
    dxgiOutput1->DuplicateOutput(gDevice, &gOutputDuplication);

    // Cleanup
    dxgiOutput1->Release();
    dxgiOutput->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();

    return true;
}

void DXGI::CaptureDesktopDXGI() {

    while (!globals.shutdown) {

        IDXGIResource* desktopResource = nullptr;
        DXGI_OUTDUPL_FRAME_INFO frameInfo = {};

        // Try to acquire next frame
        HRESULT hr = gOutputDuplication->AcquireNextFrame(1000, &frameInfo, &desktopResource);
        if (FAILED(hr)) {
            std::cerr << xorstr_("Failed to acquire next frame") << std::endl;
            continue;
        }

        // Query for ID3D11Texture2D
        ID3D11Texture2D* desktopImageTex = nullptr;
        desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&desktopImageTex));
        desktopResource->Release();

        // Get metadata to create Mat
        D3D11_TEXTURE2D_DESC desc;
        desktopImageTex->GetDesc(&desc);

        // Create staging texture
        ID3D11Texture2D* stagingTexture = nullptr;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        desc.BindFlags = 0;
        desc.MiscFlags = 0;
        gDevice->CreateTexture2D(&desc, nullptr, &stagingTexture);

        if (desktopImageTex == 0 || stagingTexture == 0) {
            continue;
        }

        // Copy to staging texture
        gContext->CopyResource(stagingTexture, desktopImageTex);

        // Map resource
        D3D11_MAPPED_SUBRESOURCE resource;
        gContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &resource);

        // Create OpenCV Mat
        cv::Mat desktopImage(desc.Height, desc.Width, CV_8UC4, resource.pData, resource.RowPitch);

        //cv::Mat frameCopy = desktopImage.clone();

        // Unmap and release
        gContext->Unmap(stagingTexture, 0);
        stagingTexture->Release();
        desktopImageTex->Release();
        gOutputDuplication->ReleaseFrame();

        // Swap buffers
        globals.capture.desktopMutex_.lock();
        globals.capture.desktopMat = desktopImage;
        globals.capture.desktopMutex_.unlock();

        if (!globals.capture.initDims) {
			globals.capture.initDims = true;
			globals.capture.desktopWidth = desktopImage.cols;
			globals.capture.desktopHeight = desktopImage.rows;
            globals.capture.desktopCenterX = globals.capture.desktopWidth / 2.0f;
			globals.capture.desktopCenterY = globals.capture.desktopHeight / 2.0f;
        }

        //imshow("output", frameCopy); // Debug window
        //cv::waitKey(1);

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
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
    int imageCenterX = image.cols / 2;
    int imageCenterY = image.rows / 2;

    float minDistance = std::numeric_limits<float>::max();
    cv::Point2f closestDetectionCenter(imageCenterX, imageCenterY);
    bool found = false;

    for (const auto& detection : detections) {
        // Original logic when forceHitbox is true, or when targeting closest (2)
        if (targetClass == 2 || (forceHitbox && detection.class_id == targetClass) ||
            (!forceHitbox && (detection.class_id == targetClass || (!found && (detection.class_id == 0 || detection.class_id == 1))))) {

            cv::Point2f detectionCenter(detection.box.x + detection.box.width / 2.0f,
                detection.box.y + detection.box.height / 2.0f);
            float distance = cv::norm(detectionCenter - cv::Point2f(imageCenterX, imageCenterY));

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
    float fovRad = fov * std::numbers::pi / 180.0f;

    // Calculate the number of pixels per degree
    // This uses half the screen width since FOV is horizontal
    float pixelsPerDegree = (float)image.cols / fov;

    // Convert pixel offsets to angles (in degrees)
    float angleX = pixelOffsetX / pixelsPerDegree;
    float angleY = pixelOffsetY / pixelsPerDegree;

    // Calculate vertical FOV based on aspect ratio
    float aspectRatio = (float)image.cols / image.rows;
    float verticalFov = 2.0f * atan(tan(fovRad / 2.0f) / aspectRatio) * 180.0f / std::numbers::pi;

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

cv::Mat DXGI::normalizeIconSize(const cv::Mat& icon) {
    cv::Mat resized;
    cv::resize(icon, resized, cv::Size(64, 64), 0, 0, cv::INTER_AREA);
    return resized;
}

cv::Mat DXGI::preprocessIcon(const cv::Mat& icon) {
    cv::Mat gray;
    cv::cvtColor(icon, gray, cv::COLOR_BGR2GRAY);

    // Remove potential defuser icon area first
    int defuserSize = gray.rows / 2.5;  // Assume defuser icon is about 1/4 of the icon height
    cv::rectangle(gray, cv::Rect(gray.cols - defuserSize, gray.rows - defuserSize, defuserSize, defuserSize), cv::Scalar(0), cv::FILLED);

    // Then apply adaptive thresholding to handle brightness variations
    cv::Mat binary;
    cv::adaptiveThreshold(gray, binary, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, 2);

    return binary;
}

IconHash DXGI::hashIcon(const cv::Mat& icon) {
    cv::Mat preprocessed = preprocessIcon(icon);
    IconHash hash;

    int idx = 0;
    for (int y = 0; y < preprocessed.rows; y += 4) {
        for (int x = 0; x < preprocessed.cols; x += 4) {
            cv::Rect block(x, y, 4, 4);
            double avgIntensity = cv::mean(preprocessed(block))[0];
            hash[idx++] = (avgIntensity > 127);

            // Calculate horizontal gradient
            if (x < preprocessed.cols - 4) {
                cv::Rect nextBlock(x + 4, y, 4, 4);
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

    while (!globals.shutdown) {

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

            settings.aimbotData.correctionX = corrections[0] * settings.activeState.sensMultiplier_SensOnly[0];
            settings.aimbotData.correctionY = corrections[1] * settings.activeState.sensMultiplier_SensOnly[1];

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

    while (!globals.shutdown) {
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

        // Handle timing and state
        auto currentTime = std::chrono::steady_clock::now();

        // Process burst timing
        if (isBurstActive) {
            auto burstElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - burstStartTime).count();
            if (burstElapsed >= settings.aimbotData.triggerSettings.burstDuration) {
                utils.pressMouse1(false);
                isBurstActive = false;
                justFired = true;
                firstFrame = true;
            }
            else {
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
        if (lastRadius != radius) {
            const int centerX = globals.capture.desktopWidth / 2;
            const int centerY = globals.capture.desktopHeight / 2;
            roi = cv::Rect(
                std::max(0, centerX - radius),
                std::max(0, centerY - radius),
                std::min(radius * 2, globals.capture.desktopWidth - (centerX - radius)),
                std::min(radius * 2, globals.capture.desktopHeight - (centerY - radius))
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
                if (settings.aimbotData.triggerSettings.burstDuration > 0) {
                    // Burst mode
                    utils.pressMouse1(true);
                    isBurstActive = true;
                    burstStartTime = currentTime;
                }
                else {
                    // Single click mode - optimized for minimum latency
                    utils.pressMouse1(true);
                    std::this_thread::sleep_for(std::chrono::microseconds(300));
                    utils.pressMouse1(false);
                    justFired = true;
                    firstFrame = true;
                }

                lastTriggerTime = currentTime;
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
    rectangle(src, roi1, cv::Scalar(255, 0, 0), 2); // Blue rectangle around ROI 1  // Keeps crashing here
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

void debugHammingDistances(const IconHash& hash) {
    std::vector<std::pair<std::string, float>> distances;

    for (const auto& pair : operatorHashes) {
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

    cv::Mat normalizedIcon = normalizeIconSize(src);
    IconHash hash = hashIcon(normalizedIcon);

    /*std::cout << xorstr_("Hash: ") << hash << std::endl;
	debugHammingDistances(hash);*/

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

        if (bestMatchPercentage <= 12.5f) {  // Adjust between 1-5% based on testing
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
    static const int bottomRightWidth = MASK_SIZE * 0.45;
    static const int bottomRightHeight = MASK_SIZE * 0.27;
    static const int topLeftWidth = MASK_SIZE * 0.2;
    static const int topLeftHeight = MASK_SIZE * 0.4;

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
    //static auto lastSaveTime = std::chrono::steady_clock::now();

    if (src.empty()) {
        return;
    }

    int activeBoxIndex = -1;
    cv::Mat activeWeaponIcon;
    double maxColorScore = 0.0;

    const int targetHue = 102;  // 204 / 2 for OpenCV's 0-180 scale
    const int targetSat = 212;  // 83 * 255 / 100
    const int targetVal = 144;

    // Detect which box is active (has blue background)
    for (int i = 0; i < weaponBoxes.size(); ++i) {
        cv::Mat boxROI = src(weaponBoxes[i]);
        cv::Mat hsv;
        cv::cvtColor(boxROI, hsv, cv::COLOR_BGR2HSV);

        double colorScore = 0.0;
        double totalPixels = boxROI.total();
        int matchingPixels = 0;

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

                if (similarity > 0.8) { // Adjust this threshold as needed
                    colorScore += similarity;
                    matchingPixels++;
                }
            }
        }

        // Normalize the color score
        colorScore /= totalPixels;

        // Factor in the proportion of matching pixels
        double matchingRatio = matchingPixels / totalPixels;
        colorScore *= matchingRatio;

        // Update if this is the highest color score so far
        if (colorScore > maxColorScore) {
            maxColorScore = colorScore;
            activeBoxIndex = i;
            activeWeaponIcon = boxROI.clone();
        }

        cv::Scalar boxColor = (i == activeBoxIndex) ? cv::Scalar(0, 255, 0) : cv::Scalar(0, 0, 255);
        cv::rectangle(src, weaponBoxes[i], boxColor, 2);
    }

    const double MIN_WEIGHTED_SCORE = 0.00001;

    // If an active box is found and meets the minimum blue threshold, detect the weapon
    if (activeBoxIndex != -1 && maxColorScore >= MIN_WEIGHTED_SCORE) {
        std::string detectedWeapon = detectWeaponTypeWithMask(activeWeaponIcon);

        settings.activeState.weaponOffOverride = false;

        if (detectedWeapon == xorstr_("Unknown Weapon")) {
            return; // Don't set weapon if it's an unknown weapon
		}

        // Find the matching weapon in the weapondata vector
        int weaponIndex = -1;
        for (size_t i = 0; i < settings.characters[settings.activeState.selectedCharacterIndex].weapondata.size(); ++i) {
            if (settings.characters[settings.activeState.selectedCharacterIndex].weapondata[i].weaponname == detectedWeapon) {
                weaponIndex = static_cast<int>(i);
                break;
            }
        }

        // Set the selectedPrimary if a matching weapon is found
        if (weaponIndex != -1) {
            settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0] = weaponIndex;
            settings.characters[settings.activeState.selectedCharacterIndex].weapondata[settings.characters[settings.activeState.selectedCharacterIndex].selectedweapon[0]].rapidfire = settings.characters[settings.activeState.selectedCharacterIndex].weapondata[weaponIndex].rapidfire;
            settings.activeState.weaponDataChanged = true;
        }
        else {
            std::cout << xorstr_("Warning: Detected weapon not found in weapondata: ") << detectedWeapon << std::endl;
        }

        //std::cout << g.characterinfo.selectedPrimary;

        /*auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - lastSaveTime).count() >= 5) {
            std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::string filename = "weapon_icon_" + std::to_string(time) + ".png";
            cv::imwrite(filename, activeWeaponIcon);
            std::cout << "Saved weapon icon: " << filename << std::endl;
            lastSaveTime = now;
        }*/
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
        cv::Point2f pos;
        float area;
        int lastSeen;
        float confidence;
    };
    static std::vector<SmoothTarget> trackedTargets;
    static int frameCount = 0;
    const int maxTrackAge = settings.aimbotData.colourAimbotSettings.maxTrackAge;  // Maximum frames to keep tracking a disappeared target
    const float smoothingFactor = PERCENT(settings.aimbotData.colourAimbotSettings.trackSmoothingFactor);  // Higher = more smoothing, range 0-1

    frameCount++;

    if (!simdInitialized) {
        hasAVX512 = globals.startup.avx == 2;
        hasAVX2 = globals.startup.avx >= 1;
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
        cv::Point center;
        int area;
        cv::Rect bounds;
        float density;
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
        comp.area = static_cast<float>(area);
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
            newTrack.area = clusters[i].area;
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
        const float scaledX = (targetPoint.x * 2) - src.cols / 2;
        const float scaledY = (targetPoint.y * 2) - src.rows / 2;

        settings.aimbotData.correctionX = (scaledX / pixelsPerDegree) * fovScale * sens[0];
        settings.aimbotData.correctionY = (scaledY / pixelsPerDegree) * fovScale * sens[1];

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