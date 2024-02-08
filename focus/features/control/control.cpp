#include "control.hpp"

Utils ut;

void Control::driveMouse() {

	static bool complete = false;
	Settings currwpn;
	static int maxInstructions = 0;
	std::vector<std::vector<int>> weaponData = currwpn.values;

	while (!g.shutdown) {
		// Check if the selected weapon has changed
		if (currwpn.values != g.selectedWeapon.values) {
			currwpn = g.selectedWeapon;
			weaponData = currwpn.values;

			// Print the new weapon data
			for (auto const& data : weaponData) {
				std::cout << "{" << data[0] << ", " << data[1] << ", " << data[2] << "}," << std::endl;
			}

			// Update maxInstructions
			maxInstructions = weaponData.size();
		}

		if (GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON) && !complete) {
			for (int index = 0; index < maxInstructions; index++) {
				auto& instruction = weaponData[index];
				int x = instruction[0], y = instruction[1], duration = instruction[2];
				auto currtime = std::chrono::high_resolution_clock::now();
				float int_timer = 0;

				while (int_timer < duration / 1000.f && GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON)) {
					auto elapsed = std::chrono::high_resolution_clock::now() - currtime;
					int_timer = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0f;

					ms.mouse_move(0, x, y, 0);
					//do autofire here

					for (auto const& data : weaponData) {
						std::cout << "{" << x << ", " << y << ", " << duration << "}," << std::endl;
					}

					ut.preciseSleep(0.01);
				}

				if (index == maxInstructions - 1) {
					complete = true;
				}
			}
		}
		else if (!GetAsyncKeyState(VK_LBUTTON) || !GetAsyncKeyState(VK_RBUTTON)) {
			complete = false;
		}
	}
}