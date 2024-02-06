#pragma once

#include "includes.hpp"

#include "features/menu/menu.hpp"
#include "features/control/control.hpp"

Control ctr;
Menu mn;
Settings cfg;

TextEditor editor;

#define WINDOWFLAGS (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool isedited(const std::string& original, const std::string& changed) {
	if (original == changed) {
		return false;
	}
	else {
		return true;
	}
}

void Gui()
{	
	bool inverseShutdown = !g.shutdown;

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
	ImGui::Begin("Focus", &inverseShutdown, WINDOWFLAGS);

	if (ImGui::BeginTabBar("##TabBar"))
	{
		if (ImGui::BeginTabItem("Weapon")) {
			g.selectedWeapon = g.weapons[g.selectedItem];

			if (mn.comboBox("Weapon", g.selectedItem, g.weapons)) {
				cfg.readSettings("weapons.json", g.weapons, true);
			}

			ImGui::Text(g.selectedWeapon.name.c_str());

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Edit")) {

			auto cpos = editor.GetCursorPosition();
			bool ro = editor.IsReadOnly();

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Save", "Ctrl-S", nullptr, !ro))
					{
						auto textToSave = editor.GetText();
						if (mn.saveTextToFile("weapons.json", textToSave)) {
							std::cout << "Saved File" << std::endl;
							g.weaponsText = mn.readTextFromFile("weapons.json");
						}
						else {
							std::cout << "Failed to Save File" << std::endl;
						}
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit"))
				{
					if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
						editor.SetReadOnly(ro);
					ImGui::Separator();

					if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
						editor.Undo();
					if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
						editor.Redo();

					ImGui::Separator();

					if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
						editor.Copy();
					if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
						editor.Cut();
					if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
						editor.Delete();
					if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
						editor.Paste();

					ImGui::Separator();

					if (ImGui::MenuItem("Select all", nullptr, nullptr))
						editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View"))
				{
					if (ImGui::MenuItem("Dark palette"))
						editor.SetPalette(TextEditor::GetDarkPalette());
					if (ImGui::MenuItem("Light palette"))
						editor.SetPalette(TextEditor::GetLightPalette());
					if (ImGui::MenuItem("Retro blue palette"))
						editor.SetPalette(TextEditor::GetRetroBluePalette());
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
				editor.IsOverwrite() ? "Ovr" : "Ins",
				editor.CanUndo() ? "*" : " ",
				isedited(g.weaponsText, editor.GetText()) ? "*" : " ",
				editor.GetLanguageDefinition().mName.c_str(), "weapons.json");

			ImGui::Spacing();
			ImGui::Spacing();

			editor.Render("TextEditor");

			ImGuiIO& io = ImGui::GetIO();

			// Check if Ctrl key is pressed and 'S' key is pressed
			if (io.KeyCtrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_S)))
			{
				auto textToSave = editor.GetText();
				if (mn.saveTextToFile("weapons.json", textToSave)) {
					std::cout << "Saved File" << std::endl;
					g.weaponsText = mn.readTextFromFile("weapons.json");
				}
				else {
					std::cout << "Failed to Save File" << std::endl;
				}
			}

			ImGui::EndTabItem(); 
		}

		ImGui::EndTabBar();
	}

	ImGui::End();

	g.shutdown = !inverseShutdown;
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
	GLFWwindow* window = glfwCreateWindow(1, 1, "Focus", nullptr, nullptr);
	if (window == nullptr)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	auto hwnd = glfwGetWin32Window(window);
	SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);

	ImVec4 clear_color = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().IniFilename = NULL;
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	//
	static const char* fileToEdit = "weapons.json";
	g.weaponsText = mn.readTextFromFile(fileToEdit);
	editor.SetText(g.weaponsText);

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

    cfg.readSettings("weapons.json", g.weapons, false);

	cfg.printSettings(g.weapons);

	std::thread driveMouseThread(&Control::driveMouse, &ctr);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Gui();

		if (g.shutdown) {
			glfwSetWindowShouldClose(window, true);
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
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

		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	g.shutdown = true;

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
