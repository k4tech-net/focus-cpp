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
        globals.desktopMutex_.lock();
        globals.desktopMat = desktopImage;
        globals.desktopMutex_.unlock();

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
    if (settings.mode == xorstr_("Game") && settings.game == xorstr_("Rust")) {
        mouseX = (angleX / 360.0f) * constants.RUST360DIST;
        mouseY = (angleY / 360.0f) * constants.RUST360DIST;
    }
    else {
        mouseX = (angleX / 360.0f) * constants.SIEGE360DIST;
        mouseY = (angleY / 360.0f) * constants.SIEGE360DIST;
    }

    mouseX *= settings.fovSensitivityModifier;
    mouseY *= settings.fovSensitivityModifier;

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

    // Pre-calculate ROI values
    const float cropRatioX = 0.25f;
    const float cropRatioY = 0.25f;
    const float cropRatioWidth = 0.5f;
    const float cropRatioHeight = 0.5f;

    std::unique_ptr<YoloInferencer> inferencer;

    while (!globals.shutdown) {

        if (settings.test) {

            // FPS tracking variables
            static int frameCount = 0;
            static auto lastFpsTime = std::chrono::high_resolution_clock::now();
            static float currentFps = 0.0f;

            if (globals.desktopMat.empty()) {
                std::this_thread::sleep_for(std::chrono::microseconds(100));
                continue;
            }

            cv::Mat croppedImage;
            {
                std::lock_guard<std::mutex> lock(globals.desktopMutex_);

                // Calculate ROI directly from desktop dimensions
                const int screenWidth = globals.desktopMat.cols;
                const int screenHeight = globals.desktopMat.rows;
                const cv::Rect roi(
                    static_cast<int>(cropRatioX * screenWidth),
                    static_cast<int>(cropRatioY * screenHeight),
                    static_cast<int>(cropRatioWidth * screenWidth),
                    static_cast<int>(cropRatioHeight * screenHeight)
                );

                // Actually crop the image (this was missing in original code)
                croppedImage = globals.desktopMat(roi).clone();
            }

            if (croppedImage.empty()) {
                continue;
            }

            overwatchDetector(croppedImage);

            cv::imshow(xorstr_("Test"), croppedImage);
            cv::waitKey(1);

            // Only increment frame count after successful processing
            frameCount++;

            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFpsTime).count();

            // Update FPS every 500ms for smoother readings
            if (elapsed >= 500) {
                currentFps = (frameCount * 1000.0f) / elapsed;  // Convert to per-second rate
                printf("FPS: %.2f\n", currentFps);  // Format to 2 decimal places
                frameCount = 0;
                lastFpsTime = currentTime;
            }

            //std::this_thread::sleep_for(std::chrono::microseconds(500));
            continue;
        }

        if (!settings.aimbotData.enabled) {
            if (aimbotInit) {
                inferencer.reset();
                aimbotInit = false;
			}
            settings.aimbotData.correctionX = 0;
            settings.aimbotData.correctionY = 0;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }

        if (!aimbotInit) {
            if (settings.aimbotData.provider == 1) {
                provider = xorstr_("CUDA");
            }
            else {
				provider = xorstr_("CPU");
            }

            inferencer = std::make_unique<YoloInferencer>(modelPath, logid, provider);
            aimbotInit = true;
        }

        if (!aimbotInit) {
            continue;
        }

        cv::Mat croppedImage;
        {
            std::lock_guard<std::mutex> lock(globals.desktopMutex_);
            if (globals.desktopMat.empty()) {
                continue;
            }

            // Calculate ROI directly from desktop dimensions
            const int screenWidth = globals.desktopMat.cols;
            const int screenHeight = globals.desktopMat.rows;
            const cv::Rect roi(
                static_cast<int>(cropRatioX * screenWidth),
                static_cast<int>(cropRatioY * screenHeight),
                static_cast<int>(cropRatioWidth * screenWidth),
                static_cast<int>(cropRatioHeight * screenHeight)
            );

            // Convert BGRA to BGR during the crop operation
            cv::cvtColor(globals.desktopMat(roi), croppedImage, cv::COLOR_BGRA2BGR);
        }

        if (croppedImage.empty()) {
            continue;
        }

		std::vector<Detection> detections = inferencer->infer(croppedImage, static_cast<float>(settings.aimbotData.confidence) / 100.0f, 0.5);

        if (detections.empty()) {
			continue;
		}

        std::vector<float> corrections = calculateCorrections(croppedImage, detections, settings.aimbotData.hitbox, settings.fov, settings.aimbotData.forceHitbox);

        if (corrections[0] == 0 || settings.aimbotData.percentDistance == 0) {
            settings.aimbotData.correctionX = corrections[0];
        }
        else {
            settings.aimbotData.correctionX = corrections[0] * (static_cast<float>(settings.aimbotData.percentDistance) / 100.0f);
        }

		if (corrections[1] == 0 || settings.aimbotData.percentDistance == 0) {
            settings.aimbotData.correctionY = corrections[1];
		}
		else {
            settings.aimbotData.correctionY = corrections[1] * (static_cast<float>(settings.aimbotData.percentDistance) / 100.0f);
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

    if (((settings.mode == xorstr_("Character") && settings.characters[settings.selectedCharacterIndex].options[4]) ||
        (settings.mode == xorstr_("Game") && settings.game == xorstr_("Siege") && settings.characters[settings.selectedCharacterIndex].options[1])) &&
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
        settings.weaponOffOverride = true;
    }
    else {
        //std::cout << "Active ROI: " << activeROI << std::endl;
        settings.weaponOffOverride = false;
        if (activeROI == 2) {
            settings.isPrimaryActive = true;
            settings.weaponDataChanged = true;
        }
        else if (activeROI == 3) {
            settings.isPrimaryActive = false;
            settings.weaponDataChanged = true;
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

    //std::cout << xorstr_("Hash: ") << hash << std::endl;
	//debugHammingDistances(hash);

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

        if (bestMatchPercentage <= 10.0f) {  // Adjust between 1-5% based on testing
            operatorFound = true;
        }
    }

    if (operatorFound) {
        int characterIndex = utils.findCharacterIndex(detectedOperator);
        if (characterIndex != -1) {
            settings.selectedCharacterIndex = characterIndex;
            settings.weaponDataChanged = true;
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

        settings.weaponOffOverride = false;

        if (detectedWeapon == xorstr_("Unknown Weapon")) {
            return; // Don't set weapon if it's an unknown weapon
		}

        // Find the matching weapon in the weapondata vector
        int weaponIndex = -1;
        for (size_t i = 0; i < settings.characters[settings.selectedCharacterIndex].weapondata.size(); ++i) {
            if (settings.characters[settings.selectedCharacterIndex].weapondata[i].weaponname == detectedWeapon) {
                weaponIndex = static_cast<int>(i);
                break;
            }
        }

        // Set the selectedPrimary if a matching weapon is found
        if (weaponIndex != -1) {
            settings.characters[settings.selectedCharacterIndex].selectedweapon[0] = weaponIndex;
            settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]].rapidfire = settings.characters[settings.selectedCharacterIndex].weapondata[weaponIndex].rapidfire;
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
        settings.weaponOffOverride = true;
    }
}

void DXGI::overwatchDetector(cv::Mat& src) {
    // Pre-allocate matrices and vectors
    static cv::Mat downsampledSrc, hsvImage, mask, filledMask;
    static std::vector<std::vector<cv::Point>> contours;

    // Create kernels for morphological operations
    static cv::Mat closeKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 10));
    static cv::Mat dilateKernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(10, 10)); // Larger kernel for connecting gaps

    // Downsample
    cv::resize(src, downsampledSrc, cv::Size(src.cols / 2, src.rows / 2), 0, 0, cv::INTER_NEAREST);

    // Convert to HSV
    cv::cvtColor(downsampledSrc, hsvImage, cv::COLOR_BGR2HSV);

    // Color detection for magenta
    cv::inRange(hsvImage, cv::Scalar(145, 100, 100), cv::Scalar(165, 255, 255), mask);

    // Connect nearby regions
    cv::morphologyEx(mask, filledMask, cv::MORPH_CLOSE, closeKernel);  // Initial closing to clean noise
    cv::dilate(filledMask, filledMask, dilateKernel);  // Dilate to connect nearby regions
    cv::morphologyEx(filledMask, filledMask, cv::MORPH_CLOSE, closeKernel);  // Final closing to smooth edges

    // Find contours
    contours.clear();
    cv::findContours(filledMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Centers and FOV calculations
    const int imageCenterX = downsampledSrc.cols / 2;
    const int imageCenterY = downsampledSrc.rows / 2;
    float minDistance = std::numeric_limits<float>::max();
    bool targetFound = false;
    cv::Point targetPoint;
    cv::Rect targetBox;

    // Find closest bounding box
    for (const auto& contour : contours) {
        cv::Rect box = cv::boundingRect(contour);
        cv::Point boxCenter(box.x + box.width / 2, box.y + box.height / 2);
        float distance = cv::norm(boxCenter - cv::Point(imageCenterX, imageCenterY));

        if (distance < minDistance) {
            minDistance = distance;
            // Aim point is slightly above center
            targetPoint = cv::Point(boxCenter.x, box.y + box.height * 0.2);
            targetBox = box;
            targetFound = true;
        }
    }

    if (targetFound) {
        // Calculate FOV-based corrections
        const float effectiveFOV = settings.fov;
        const float fovRad = effectiveFOV * std::numbers::pi / 180.0f;
        const float pixelsPerDegree = static_cast<float>(downsampledSrc.cols) / effectiveFOV;

        // Calculate aspect ratio and vertical FOV
        const float aspectRatio = static_cast<float>(downsampledSrc.cols) / downsampledSrc.rows;
        const float verticalFov = 2.0f * atan(tan(fovRad * 0.5f) / aspectRatio) * 180.0f / std::numbers::pi;

        // Calculate pixel offsets
        const float pixelOffsetX = targetPoint.x - imageCenterX;
        const float pixelOffsetY = targetPoint.y - imageCenterY;

        // Convert to angles
        const float angleX = pixelOffsetX / pixelsPerDegree;
        const float angleY = (pixelOffsetY / pixelsPerDegree) * (effectiveFOV / verticalFov);

        // Calculate final mouse movements
        settings.aimbotData.correctionX = (angleX / 360.0f) * constants.OW360DIST * 2;
        settings.aimbotData.correctionY = (angleY / 360.0f) * constants.OW360DIST * 2;

        // Debug visualization
#ifdef _DEBUG
// Show both original and filled masks
        cv::imshow("Original Mask", mask);
        cv::imshow("Filled Mask", filledMask);

        // Draw the box (scaled back up to original size)
        cv::rectangle(src,
            cv::Rect(targetBox.x * 2, targetBox.y * 2, targetBox.width * 2, targetBox.height * 2),
            cv::Scalar(0, 255, 0), 2);

        // Draw aim point
        cv::circle(src, cv::Point(targetPoint.x * 2, targetPoint.y * 2), 3, cv::Scalar(0, 0, 255), -1);

        // Draw line from center to aim point
        cv::line(src, cv::Point(src.cols / 2, src.rows / 2),
            cv::Point(targetPoint.x * 2, targetPoint.y * 2), cv::Scalar(255, 0, 0), 1);
#endif
    }
    else {
        settings.aimbotData.correctionX = 0;
        settings.aimbotData.correctionY = 0;
    }
}