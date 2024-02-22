#pragma once

#include "includes.hpp"

#include "features/control/control.hpp"
#include "features/menu/menu.hpp"

Control ctr;
Menu mn;
TextEditor editor;

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{	
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

	// Create window with graphics context
	g.window = glfwCreateWindow(1, 1, "Focus", nullptr, nullptr);
	if (g.window == nullptr)
		return 1;
	glfwMakeContextCurrent(g.window);
	glfwSwapInterval(1); // Enable vsync

	auto hwnd = glfwGetWin32Window(g.window);
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	//io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	g.editor.jsonFiles = ut.scanCurrentDirectoryForJsonFiles();

	ImGui_ImplGlfw_InitForOpenGL(g.window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	std::thread startUpCheckThread(&Utils::startUpChecksRunner, &ut);

	bool startupchecks = true;
	auto startuptimer = std::chrono::high_resolution_clock::now();

	while (startupchecks) {
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		mn.startupchecks_gui();

		auto current_time = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(current_time - startuptimer).count();

		if (elapsed_time >= 5 && g.startup.hasFinished) {
			if (g.startup.passedstartup) {
				startupchecks = false;
			}
			else {
				g.shutdown = true;
			}
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(g.window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(g.window);
	}

	startUpCheckThread.join();

	std::thread driveMouseThread(&Control::driveMouse, &ctr);

	while (!glfwWindowShouldClose(g.window)) {
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		mn.gui();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(g.window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}

		glfwSwapBuffers(g.window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	g.shutdown = true;

	driveMouseThread.join();

	glfwDestroyWindow(g.window);
	glfwTerminate();

	ms.mouse_close();

	return 0;
}
