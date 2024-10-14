#pragma once

#include <iostream>
#include <dxgi1_2.h>
#include <d3d11.h>
#include <d3dcompiler.h>
   
#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include "Windows.h"
#include <iostream>

#include "../driver/mouse.hpp"
#include "../driver/keyboard.hpp"
#include "../settings/settings.hpp"
#include "../YoloV8-ONNXRuntime-CPP/engine.hpp"
#include "weaponmasks.hpp"

#include <onnxruntime_cxx_api.h>

class DXGI
{
public:
	bool InitDXGI();
	void CaptureDesktopDXGI();
	void CleanupDXGI();
	void aimbot();
	void detectWeaponR6(cv::Mat& src, double hysteresisThreshold, double minActiveAreaThreshold);
	void detectWeaponRust(cv::Mat& src);
	std::string detectWeaponTypeWithMask(const cv::Mat& weaponIcon);
	void initializeRustDetector(cv::Mat& src);

private:
	// DXGI variables
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gContext = nullptr;
	IDXGIOutputDuplication* gOutputDuplication = nullptr;
	double prevPrimaryArea1 = 0, prevPrimaryArea2 = 0, prevPrimaryArea3 = 0;
	double prevSecondaryArea1 = 0, prevSecondaryArea2 = 0, prevSecondaryArea3 = 0;

	struct BoxPercentage {
		float x, y, width, height;
	};

	std::vector<ProcessedMask> processedMasks;
	std::vector<cv::Rect> weaponBoxes;
};