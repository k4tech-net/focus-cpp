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

#include "../mouse_driver/mouse.hpp"
#include "../settings/settings.hpp"
#include "../yoloV8-Engine/engine.hpp"

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

private:
	// DXGI variables
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gContext = nullptr;
	IDXGIOutputDuplication* gOutputDuplication = nullptr;
	double prevPrimaryArea1 = 0, prevPrimaryArea2 = 0, prevPrimaryArea3 = 0;
	double prevSecondaryArea1 = 0, prevSecondaryArea2 = 0, prevSecondaryArea3 = 0;
};