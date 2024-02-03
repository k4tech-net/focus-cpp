#include <iostream>
#include <Windows.h>

#include "mouse_driver/mouse.hpp"

#include "hello_imgui/hello_imgui.h"
#include "imgui.h"
#include "imgui_internal.h"

void Gui()
{
	ImGui::Text("Hello");
	ImGui::ShowIDStackToolWindow();
}


int main()
{	
	HelloImGui::RunnerParams runnerParams;
	runnerParams.callbacks.ShowGui = Gui;

	//runnerParams.useImGuiTestEngine = true;

	HelloImGui::Run(runnerParams);
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
