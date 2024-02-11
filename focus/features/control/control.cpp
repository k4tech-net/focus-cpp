#include "control.hpp"

Utils ut;

void pressLKey(bool press) {
	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = 0; // Ignored for scan codes
	input.ki.wScan = MapVirtualKey('L', MAPVK_VK_TO_VSC); // Get scan code for 'L' key
	input.ki.dwFlags = press ? 0 : KEYEVENTF_KEYUP; // Set KEYEVENTF_KEYUP flag to release the key
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.dwFlags |= KEYEVENTF_SCANCODE; // Set the scan code flag

	SendInput(1, &input, sizeof(INPUT));
}

void Control::driveMouse() {

	static bool complete = false;
	Settings currwpn;
	static int maxInstructions = 0;
	static int cycles = 0;
	static bool sendXMovement = false;

	while (!g.shutdown) {
		// Check if the selected weapon has changed
		if (currwpn.values != g.selectedWeapon.values) {
			currwpn = g.selectedWeapon;

			// Print the new weapon data
			for (auto const& data : currwpn.values) {
				std::cout << "{" << data[0] << ", " << data[1] << ", " << data[2] << "}," << std::endl;
			}

			// Update maxInstructions
			maxInstructions = currwpn.values.size();
		}

		while (GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON) && !complete) {
			for (int index = 0; index < maxInstructions; index++) {
				auto& instruction = currwpn.values[index];
				int x = instruction[0], y = instruction[1], duration = instruction[2];
				auto currtime = std::chrono::high_resolution_clock::now();
				float int_timer = 0;

				while (int_timer < duration / 1000.f && GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON)) {
					auto elapsed = std::chrono::high_resolution_clock::now() - currtime;
					int_timer = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0f;

					sendXMovement = (currwpn.xdeadtime > 0) && (cycles % currwpn.xdeadtime) == 0;
					
					if (sendXMovement) {
						ms.mouse_move(0, x, y, 0);
					}
					else {
						ms.mouse_move(0, 0, y, 0);
					}
					
					if (currwpn.autofire && cycles >= 10) {
						// Toggle pressing and releasing of L key
						static bool flipFlop = false;
						pressLKey(flipFlop);
						flipFlop = !flipFlop;
					}

					for (auto const& data : currwpn.values) {
						std::cout << "{" << x << ", " << y << ", " << duration << "}," << std::endl;
					}

					ut.preciseSleep(0.01);

					cycles++;
				}

				if (index == maxInstructions - 1) {
					complete = true;
				}
			}
		}
		
		if (!GetAsyncKeyState(VK_LBUTTON) || !GetAsyncKeyState(VK_RBUTTON)) {
			complete = false;
			cycles = 0;
		}

		ut.preciseSleep(0.0005);
	}
}