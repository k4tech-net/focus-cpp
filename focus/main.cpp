#pragma once

#include "includes.hpp"

#include "features/menu/menu.hpp"
#include "features/control/control.hpp"

Control ctr;
Menu mn;
Settings cfg;	

void Gui()
{
	ImGuiTheme::ApplyTheme(ImGuiTheme::ImGuiTheme_ImGuiColorsClassic);
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
    cfg.readSettings("weapons.json", g.weapons);

	cfg.printSettings(g.weapons);

	HelloImGui::RunnerParams runnerParams;
	runnerParams.callbacks.ShowGui = Gui; 

	runnerParams.appWindowParams.windowGeometry = { 600, 800 };

	std::thread driveMouseThread(&Control::driveMouse, &ctr);

	HelloImGui::Run(runnerParams);

	driveMouseThread.join();

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
