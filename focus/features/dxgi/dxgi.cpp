#include "dxgi.hpp"

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

cv::Mat DXGI::CaptureDesktopDXGI() {
    IDXGIResource* desktopResource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frameInfo = {};

    // Try to acquire next frame
    HRESULT hr = gOutputDuplication->AcquireNextFrame(1000, &frameInfo, &desktopResource);
    if (FAILED(hr)) {
        std::cerr << xorstr_("Failed to acquire next frame") << std::endl;
        return cv::Mat();
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
        return cv::Mat();
    }

    // Copy to staging texture
    gContext->CopyResource(stagingTexture, desktopImageTex);

    // Map resource
    D3D11_MAPPED_SUBRESOURCE resource;
    gContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &resource);

    // Define ratios for crop region
    float cropRatioX = 0.8f; // 20% from left
    float cropRatioY = 0.82f; // 20% from top
    float cropRatioWidth = 0.18f; // 60% of total width
    float cropRatioHeight = 0.14f; // 60% of total height

    // Calculate bottom right quarter
    int cropX = static_cast<int>(desc.Width * cropRatioX);
    int cropY = static_cast<int>(desc.Height * cropRatioY);
    int cropWidth = static_cast<int>(desc.Width * cropRatioWidth);
    int cropHeight = static_cast<int>(desc.Height * cropRatioHeight);

    // Create OpenCV Mat
    cv::Mat desktopImage(desc.Height, desc.Width, CV_8UC4, resource.pData, resource.RowPitch);

    cv::Rect cropRegion(cropX, cropY, cropWidth, cropHeight);

    cv::Mat croppedImage = desktopImage(cropRegion).clone(); // Clone is needed as desktopImage data will be invalidated

    // Copy data to a new Mat (since desktopImage will be invalid once we unmap)
    //Mat frameCopy = desktopImage.clone();

    // Unmap and release
    gContext->Unmap(stagingTexture, 0);
    stagingTexture->Release();
    desktopImageTex->Release();
    gOutputDuplication->ReleaseFrame();

    return croppedImage;
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

void DXGI::detectWeaponR6(cv::Mat& src, double hysteresisThreshold, double minActiveAreaThreshold) {
    // Define the region of interest (ROI) coordinates as a percentage of the frame
    float roi1XPercent = 0.02f; // X coordinate percentage for ROI 1
    float roi1YPercent = 0.06f; // Y coordinate percentage for ROI 1
    float roi1WidthPercent = 0.9f; // Width percentage for ROI 1
    float roi1HeightPercent = 0.25f; // Height percentage for ROI 1

    float roi2XPercent = 0.02f; // X coordinate percentage for ROI 2
    float roi2YPercent = 0.4f; // Y coordinate percentage for ROI 2
    float roi2WidthPercent = 0.9f; // Width percentage for ROI 2
    float roi2HeightPercent = 0.25f; // Height percentage for ROI 2

    float roi3XPercent = 0.02f; // X coordinate percentage for ROI 3
    float roi3YPercent = 0.7f; // Y coordinate percentage for ROI 3
    float roi3WidthPercent = 0.9f; // Width percentage for ROI 3
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

    cv::Vec3b secondaryTargetColour = cv::Vec3b(160, 169, 153);
    int secondaryBuffer = 25;

    // Calculate the total area of matching color in each ROI
    double primaryArea1 = 0, primaryArea2 = 0, primaryArea3 = 0;
    double secondaryArea1 = 0, secondaryArea2 = 0, secondaryArea3 = 0;

    for (int y = 0; y < rgb1.rows; ++y) {
        for (int x = 0; x < rgb1.cols; ++x) {
            cv::Vec3b pixel = rgb1.at<cv::Vec3b>(y, x);
            if (pixel[0] >= primaryTargetColour[0] - primaryBuffer && pixel[0] <= primaryTargetColour[0] + primaryBuffer &&
                pixel[1] >= primaryTargetColour[1] - primaryBuffer && pixel[1] <= primaryTargetColour[1] + primaryBuffer &&
                pixel[2] >= primaryTargetColour[2] - primaryBuffer && pixel[2] <= primaryTargetColour[2] + primaryBuffer) {
                primaryArea1++;
            }

            if (pixel[0] >= secondaryTargetColour[0] - secondaryBuffer && pixel[0] <= secondaryTargetColour[0] + secondaryBuffer &&
                pixel[1] >= secondaryTargetColour[1] - secondaryBuffer && pixel[1] <= secondaryTargetColour[1] + secondaryBuffer &&
                pixel[2] >= secondaryTargetColour[2] - secondaryBuffer && pixel[2] <= secondaryTargetColour[2] + secondaryBuffer) {
                secondaryArea1++;
            }
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

            if (pixel[0] >= secondaryTargetColour[0] - secondaryBuffer && pixel[0] <= secondaryTargetColour[0] + secondaryBuffer &&
                pixel[1] >= secondaryTargetColour[1] - secondaryBuffer && pixel[1] <= secondaryTargetColour[1] + secondaryBuffer &&
                pixel[2] >= secondaryTargetColour[2] - secondaryBuffer && pixel[2] <= secondaryTargetColour[2] + secondaryBuffer) {
                secondaryArea2++;
            }
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

            if (pixel[0] >= secondaryTargetColour[0] - secondaryBuffer && pixel[0] <= secondaryTargetColour[0] + secondaryBuffer &&
                pixel[1] >= secondaryTargetColour[1] - secondaryBuffer && pixel[1] <= secondaryTargetColour[1] + secondaryBuffer &&
                pixel[2] >= secondaryTargetColour[2] - secondaryBuffer && pixel[2] <= secondaryTargetColour[2] + secondaryBuffer) {
                secondaryArea3++;
            }
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
    if (abs(secondaryArea1 - prevSecondaryArea1) < hysteresisThreshold) {
        secondaryArea1 = prevSecondaryArea1; 
    }
    if (abs(secondaryArea2 - prevSecondaryArea2) < hysteresisThreshold) {
        secondaryArea2 = prevSecondaryArea2;
    }
    if (abs(secondaryArea3 - prevSecondaryArea3) < hysteresisThreshold) {
        secondaryArea3 = prevSecondaryArea3;
    }

    // Update previous area values
    prevPrimaryArea1 = primaryArea1;
    prevPrimaryArea2 = primaryArea2;
    prevPrimaryArea3 = primaryArea3;
    prevSecondaryArea1 = secondaryArea1;
    prevSecondaryArea2 = secondaryArea2;
    prevSecondaryArea3 = secondaryArea3;

    // Determine which ROI has the largest area of the specified color
    int activeROI = 0;

    if (((CHI.mode == xorstr_("Character") && CHI.characterOptions[4]) || 
        (CHI.mode == xorstr_("Game") && CHI.game == xorstr_("Siege") && CHI.characterOptions[1])) &&
        primaryArea1 > minActiveAreaThreshold&& primaryArea1 > primaryArea2&& primaryArea1 > primaryArea3) {

        if (secondaryArea2 > minActiveAreaThreshold && secondaryArea2 > secondaryArea1 && secondaryArea2 > secondaryArea3) {
            activeROI = 2;
            rectangle(src, roi2, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 2
        }
        else if (secondaryArea3 > minActiveAreaThreshold && secondaryArea3 > secondaryArea1 && secondaryArea3 > secondaryArea2) {
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
        CHI.weaponOffOverride = true;
    }
    else {
        //std::cout << "Active ROI: " << activeROI << std::endl;
        CHI.weaponOffOverride = false;
		if (activeROI == 2) {
			CHI.isPrimaryActive = true;
		}
		else if (activeROI == 3) {
			CHI.isPrimaryActive = false;
		}
    }
}
