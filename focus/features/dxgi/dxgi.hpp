#pragma once

#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <dxgi1_2.h>
#include <d3d11.h>

#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <fstream>
#include "Windows.h"
#include <iostream>

#include "../mouse_driver/mouse.hpp"
#include "../settings/settings.hpp"

class DXGI
{
public:
	bool InitDXGI();
	cv::Mat CaptureDesktopDXGI();
	void CleanupDXGI();
	void detectWeaponR6(cv::Mat& src, double hysteresisThreshold, double minActiveAreaThreshold);

private:
	// DXGI variables
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gContext = nullptr;
	IDXGIOutputDuplication* gOutputDuplication = nullptr;
	double prevPrimaryArea1 = 0, prevPrimaryArea2 = 0, prevPrimaryArea3 = 0;
	double prevSecondaryArea1 = 0, prevSecondaryArea2 = 0, prevSecondaryArea3 = 0;
};