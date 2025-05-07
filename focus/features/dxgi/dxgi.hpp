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
#include <numbers>

#include "../driver/mouse.hpp"
#include "../driver/keyboard.hpp"
#include "../settings/settings.hpp"
#include "../YOLOv11-ONNXRuntime-CPP/engine.hpp"
#include "../utils/utils.hpp"
#include "weaponmasks.hpp"

#include <onnxruntime_cxx_api.h>

#define PERCENT(x) (static_cast<float>(x) / 100.0f)

class DXGI
{
public:
	bool InitDXGI();
	void CaptureDesktopDXGI();
	void CleanupDXGI();
	void aimbot();
	void detectWeaponR6(cv::Mat& src, double hysteresisThreshold, double minActiveAreaThreshold);
	bool detectOperatorR6(cv::Mat& src);
	void detectWeaponRust(cv::Mat& src);
	std::string detectWeaponTypeWithMask(const cv::Mat& weaponIcon);
	void initializeRustDetector(cv::Mat& src);
	void overwatchDetector(cv::Mat& src);
	void triggerbot();

	int isOperatorScreenR6(cv::Mat& src);
	void detectAttachmentsR6(cv::Mat& src);

private:
	// DXGI variables
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gContext = nullptr;
	IDXGIOutputDuplication* gOutputDuplication = nullptr;
	double prevPrimaryArea1 = 0, prevPrimaryArea2 = 0, prevPrimaryArea3 = 0;
	double prevSecondaryArea1 = 0, prevSecondaryArea2 = 0, prevSecondaryArea3 = 0;

	IconHash hashIcon(const cv::Mat& icon, int size);
	cv::Mat normalizeIconSize(const cv::Mat& icon, int width, int height);
	cv::Mat preprocessIcon(const cv::Mat& icon);

	struct BoxPercentage {
		float x, y, width, height;
	};

	struct ColourDetection {
		cv::Point2f center;
		float area;
		cv::Rect boundingBox;
	};

	struct ROIParameters {
		float ratioX;
		float ratioY;
		float ratioWidth;
		float ratioHeight;
		float confidence;
	};

	ROIParameters optimizeOperatorDetectionROI(cv::Mat& src, float initialX, float initialY, float initialWidth, float initialHeight, float stepSize);
	float getOperatorDetectionConfidence(cv::Mat& roi);
	void runOperatorOptimiser(float x, float y, float width, float height, cv::Mat& src);
	void runAttachmentOptimiser(float x, float y, float width, float height, cv::Mat& src);
	std::vector<int> detectAttachmentsR6FromRegion(cv::Mat& attachmentRegion);
	std::vector<float> getAttachmentDetectionConfidence(cv::Mat& attachmentRegion);
	ROIParameters optimizeAttachmentDetectionROI(cv::Mat& src, float initialX, float initialY, float initialWidth, float initialHeight, float stepSize);

	std::vector<ProcessedMask> processedMasks;
	std::vector<cv::Rect> weaponBoxes;
};