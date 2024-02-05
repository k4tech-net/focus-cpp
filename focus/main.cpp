#pragma once

#include "includes.hpp"

#include "features/menu/menu.hpp"
#include "features/control/control.hpp"

Control ctr;
Menu mn;
Settings cfg;

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void Gui()
{
	ImGui::Text("Focus-cpp");

	g.selectedWeapon = g.weapons[g.selectedItem];
	
	mn.ComboBox("Weapon", g.selectedItem, g.weapons);

	ImGui::Text(g.selectedWeapon.name.c_str());

	if (GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON)) {
		ImGui::Text("buttons");
	}
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

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);

	ImGui_ImplOpenGL3_Init(glsl_version);

    cfg.readSettings("weapons.json", g.weapons);

	cfg.printSettings(g.weapons);

	std::thread driveMouseThread(&Control::driveMouse, &ctr);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Gui();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	driveMouseThread.join();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;

	//if (!mouse_open()) {
	//	printf("[-] failed to open ghub macro driver\n");
	//	return 0;
	//}

	//for (int i = 0; i < 32; i++) {
	//	Sleep(100);
	//	printf("[+] moving mouse\n");

	//	mouse_move(0, -10, 0, 0);
	//}

	//mouse_close();
}
