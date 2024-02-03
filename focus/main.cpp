#include <iostream>
#include <Windows.h>

#include "mouse_driver/mouse.hpp"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

int main()
{	

	if (!mouse_open()) {
		printf("[-] failed to open ghub macro driver\n");
		return 0;
	}


	for (int i = 0; i < 32; i++) {
		Sleep(100);
		printf("[+] moving mouse\n");

		mouse_move(0, -10, 0, 0);
	}


	mouse_close();
}
