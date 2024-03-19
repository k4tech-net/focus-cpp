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
#include "../settings/globals.hpp"

#define CHI g.characterinfo

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
	double prevArea1 = 0, prevArea2 = 0, prevArea3 = 0;
};

extern Globals g;