#include "dxgi.hpp"

Globals g;

bool DXGI::InitDXGI() {
    // Create device and context
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &gDevice, &featureLevel, &gContext);
    if (FAILED(hr)) {
        std::cerr << "Failed to create D3D11 device" << std::endl;
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
        std::cerr << "Failed to acquire next frame" << std::endl;
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

    // Copy to staging texture
    gContext->CopyResource(stagingTexture, desktopImageTex);

    // Map resource
    D3D11_MAPPED_SUBRESOURCE resource;
    gContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &resource);

    // Define ratios for crop region
    float cropRatioX = 0.8; // 20% from left
    float cropRatioY = 0.86; // 20% from top
    float cropRatioWidth = 0.18; // 60% of total width
    float cropRatioHeight = 0.1; // 60% of total height

    // Calculate bottom right quarter
    int cropX = desc.Width * cropRatioX;
    int cropY = desc.Height * cropRatioY;
    int cropWidth = desc.Width * cropRatioWidth;
    int cropHeight = desc.Height * cropRatioHeight;

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

void DXGI::detectWeapon(cv::Mat& src, double hysteresisThreshold, double minActiveAreaThreshold) {
    // Define the region of interest (ROI) coordinates as a percentage of the frame
    float roi1XPercent = 0.02; // X coordinate percentage for ROI 1
    float roi1YPercent = 0.08; // Y coordinate percentage for ROI 1
    float roi1WidthPercent = 0.9; // Width percentage for ROI 1
    float roi1HeightPercent = 0.45; // Height percentage for ROI 1

    float roi2XPercent = 0.02; // X coordinate percentage for ROI 2
    float roi2YPercent = 0.53; // Y coordinate percentage for ROI 2
    float roi2WidthPercent = 0.9; // Width percentage for ROI 2
    float roi2HeightPercent = 0.45; // Height percentage for ROI 2

    // Calculate ROI coordinates based on percentage of frame dimensions
    cv::Rect roi1(src.cols * roi1XPercent, src.rows * roi1YPercent, src.cols * roi1WidthPercent, src.rows * roi1HeightPercent);
    cv::Rect roi2(src.cols * roi2XPercent, src.rows * roi2YPercent, src.cols * roi2WidthPercent, src.rows * roi2HeightPercent);

    // Highlight the ROIs on the source image for alignment
    rectangle(src, roi1, cv::Scalar(255, 0, 0), 2); // Blue rectangle around ROI 1
    rectangle(src, roi2, cv::Scalar(255, 0, 0), 2); // Blue rectangle around ROI 2

    // Extract the ROIs from the source image
    cv::Mat roiImg1 = src(roi1);
    cv::Mat roiImg2 = src(roi2);

    // Convert ROIs to RGB for processing
    cv::Mat rgb1, rgb2;
    cvtColor(roiImg1, rgb1, cv::COLOR_BGR2RGB);
    cvtColor(roiImg2, rgb2, cv::COLOR_BGR2RGB);

    // Define the color to compare
    cv::Vec3b targetColor(15, 255, 243); // RGB(15, 255, 243)

    // Define the buffer size for color matching
    int colorBuffer = 15;

    // Calculate the total area of matching color in each ROI
    double area1 = 0, area2 = 0;
    for (int y = 0; y < rgb1.rows; ++y) {
        for (int x = 0; x < rgb1.cols; ++x) {
            cv::Vec3b pixel = rgb1.at<cv::Vec3b>(y, x);
            if (pixel[0] >= targetColor[0] - colorBuffer && pixel[0] <= targetColor[0] + colorBuffer &&
                pixel[1] >= targetColor[1] - colorBuffer && pixel[1] <= targetColor[1] + colorBuffer &&
                pixel[2] >= targetColor[2] - colorBuffer && pixel[2] <= targetColor[2] + colorBuffer) {
                area1++;
            }
        }
    }
    for (int y = 0; y < rgb2.rows; ++y) {
        for (int x = 0; x < rgb2.cols; ++x) {
            cv::Vec3b pixel = rgb2.at<cv::Vec3b>(y, x);
            if (pixel[0] >= targetColor[0] - colorBuffer && pixel[0] <= targetColor[0] + colorBuffer &&
                pixel[1] >= targetColor[1] - colorBuffer && pixel[1] <= targetColor[1] + colorBuffer &&
                pixel[2] >= targetColor[2] - colorBuffer && pixel[2] <= targetColor[2] + colorBuffer) {
                area2++;
            }
        }
    }

    // Apply hysteresis to prevent rapid changes in ROIs
    if (abs(area1 - prevArea1) < hysteresisThreshold) {
        area1 = prevArea1;
    }
    if (abs(area2 - prevArea2) < hysteresisThreshold) {
        area2 = prevArea2;
    }

    // Update previous area values
    prevArea1 = area1;
    prevArea2 = area2;

    // Determine which ROI has the largest area of the specified color
    int activeROI = 0;
    if (area1 > minActiveAreaThreshold && area1 > area2) {
        activeROI = 1;
        rectangle(src, roi1, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 1
    }
    else if (area2 > minActiveAreaThreshold && area2 > area1) {
        activeROI = 2;
        rectangle(src, roi2, cv::Scalar(0, 255, 0), 2); // Green rectangle for active ROI 2
    }

    // Print the index of the active ROI or indicate neither if both are not active
    if (activeROI == 0) {
        //std::cout << "Neither ROI is active" << std::endl;
        CHI.weaponOffOverride = true;
    }
    else {
        //std::cout << "Active ROI: " << activeROI << std::endl;
        CHI.weaponOffOverride = false;
		if (activeROI == 1) {
			CHI.isPrimaryActive = true;
		}
		else if (activeROI == 2) {
			CHI.isPrimaryActive = false;
		}
    }
}
