#pragma once

#include "includes.hpp"

#include "features/control/control.hpp"
#include "features/menu/menu.hpp"
#include "features/crypto/crypto.hpp"

#include <xorstr.hpp>

Control ctr;
Menu mn;
Crypto cr;

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
void RegisterRawInput(HWND hwnd);
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::string key = xorstr_("ASNFZ4mrze8BI0VniavN7wEjRWeJq83vASNFZ4mrze8=");
std::string clientVerificationKey = xorstr_("4783086bd5eacdea0f09c8fc6fea1642df93bbd8a314541b67a46bc4401fb55e");

int main()
{	
	//SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Focus", nullptr };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Focus", WS_POPUPWINDOW, 0, 0, 0, 0, nullptr, nullptr, wc.hInstance, nullptr);

	RegisterRawInput(hwnd);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	//io.IniFilename = NULL;

	ImGui::StyleColorsCustom();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	globals.filesystem.configFiles = ut.scanCurrentDirectoryForConfigFiles();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	#if _DEBUG
	//cv::namedWindow("output", cv::WINDOW_NORMAL);   // For debugging
	#endif

	cr.lastAuthTime.store(std::chrono::steady_clock::now());

	std::thread startUpCheckThread(&Utils::startUpChecksRunner, &ut);

	#if !_DEBUG
	std::thread watchdogThread(&Crypto::watchdog, &cr);
	#endif

	bool startupchecks = true;
	auto startuptimer = std::chrono::high_resolution_clock::now();

	while (startupchecks) {

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		mn.startupchecks_gui();

		auto current_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - startuptimer).count();

		if (elapsed_time >= 5 && globals.startup.hasFinished.load()) {
			if (globals.startup.passedstartup.load()) {
				startupchecks = false;
			}
			else {
				globals.initshutdown.store(true);
				startupchecks = false;
			}
		}

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		g_pSwapChain->Present(1, 0); // Present with vsync

		std::this_thread::sleep_for(std::chrono::microseconds(20));
	}

	startUpCheckThread.join();

	std::thread driveMouseThread(&Control::driveMouse, &ctr);
	std::thread driveAimbotThread(&Control::driveAimbot, &ctr);
	std::thread driveKeyboardThread(&Control::driveKeyboard, &ctr);
	std::thread captureDesktopThread(&DXGI::CaptureDesktopDXGI, &dx);
	std::thread aimbotThread(&DXGI::aimbot, &dx);
	std::thread triggerbotThread(&DXGI::triggerbot, &dx);

	while (!globals.done.load()) {
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				globals.done.store(true);
		}
		if (globals.done.load())
			break;

		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		HWND mainWindow = (HWND)ImGui::GetMainViewport()->PlatformHandle;
		if (mainWindow) {
			::ShowWindow(mainWindow, settings.misc.hotkeys.IsActive(HotkeyIndex::HideUiKey) ? SW_HIDE : SW_SHOW);
		}

		// Handle additional viewports (child windows)
		for (int i = 1; i < ImGui::GetPlatformIO().Viewports.Size; i++) {
			ImGuiViewport* viewport = ImGui::GetPlatformIO().Viewports[i];
			if (viewport && viewport->PlatformHandle) {
				::ShowWindow((HWND)viewport->PlatformHandle, settings.misc.hotkeys.IsActive(HotkeyIndex::HideUiKey) ? SW_HIDE : SW_SHOW);
			}
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		mn.gui();

		//if (GetAsyncKeyState(VK_RSHIFT)) {
			//if (!globals.capture.desktopMat.empty()) {
			//	globals.capture.desktopMutex_.lock();
			//	cv::Mat desktop;
			//	cv::resize(globals.capture.desktopMat, desktop, cv::Size(1920, 1080));
			//	cv::imshow("output", desktop); // Debug window
			//	globals.capture.desktopMutex_.unlock();
			//}
		//}

		if (ImGui::GetPlatformIO().Viewports.Size > 1) {
			ImGuiViewport* viewport = ImGui::GetPlatformIO().Viewports[1];
			viewport->Flags |= ImGuiViewportFlags_NoTaskBarIcon;
		}

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		g_pSwapChain->Present(1, 0); // Present with vsync

		std::this_thread::sleep_for(std::chrono::microseconds(20));
	}

	std::cout << xorstr_("Shutting Down") << std::endl;

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	globals.shutdown.store(true);

	#if !_DEBUG
	watchdogThread.join();
	#endif

	triggerbotThread.join();
	aimbotThread.join();
	captureDesktopThread.join();
	driveKeyboardThread.join();
	driveAimbotThread.join();
	driveMouseThread.join();
  
	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	ms.mouse_close();

	return 0;
}

extern "C" __declspec(dllexport) bool verification(const char* iv, const char* verificationKey) {

	std::string decryptedVerificationKey = cr.decryptDecode(verificationKey, key, iv);

	if (decryptedVerificationKey == clientVerificationKey && !globals.shutdown.load()) {
		cr.lastAuthTime.store(std::chrono::steady_clock::now());
		return true;
	}
	else {
		return false;
	}
}

// Helper functions

void RegisterRawInput(HWND hwnd) {
	RAWINPUTDEVICE rid;
	rid.usUsagePage = 0x01; // HID_USAGE_PAGE_GENERIC
	rid.usUsage = 0x02; // HID_USAGE_GENERIC_MOUSE
	rid.dwFlags = RIDEV_INPUTSINK; // Receive input even when not in focus
	rid.hwndTarget = hwnd;

	if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
		std::cerr << xorstr_("Failed to register raw input devices.") << std::endl;
	}
}

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));

	if (pBackBuffer == 0) {
		return;
	}

	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			//const int dpi = HIWORD(wParam);
			//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;
	case WM_INPUT:
		UINT dwSize;
		GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
		LPBYTE lpb = new BYTE[dwSize];
		if (lpb == nullptr) {
			return 0;
		}

		if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
			std::cerr << xorstr_("GetRawInputData does not return correct size!") << std::endl;
		}

		RAWINPUT* raw = (RAWINPUT*)lpb;
		if (raw->header.dwType == RIM_TYPEMOUSE) {
			bool lButtonDown = (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) != 0;
			bool lButtonUp = (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) != 0;
			bool rButtonDown = (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) != 0;
			bool rButtonUp = (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) != 0;

			// Handle the mouse button states
			if (raw->data.mouse.ulExtraInformation != globals.mouseinfo.marker.load(std::memory_order_relaxed)) { // Only process real inputs
				if (lButtonDown) {
					globals.mouseinfo.l_mouse_down.store(true, std::memory_order_relaxed);
				}
				if (lButtonUp) {
					globals.mouseinfo.l_mouse_down.store(false, std::memory_order_relaxed);
				}
				if (rButtonDown) {
					globals.mouseinfo.r_mouse_down.store(true, std::memory_order_relaxed);
				}
				if (rButtonUp) {
					globals.mouseinfo.r_mouse_down.store(false, std::memory_order_relaxed);
				}
			}
		}

		delete[] lpb;
		break;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
