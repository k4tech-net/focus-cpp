#include "control.hpp"

Utils ut;

void pressMouse1(bool press) {
	INPUT input;
	input.type = INPUT_MOUSE;
	input.mi.dx = 0;
	input.mi.dy = 0;
	input.mi.mouseData = 0;
	input.mi.dwFlags = press ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
	input.mi.time = 0;
	input.mi.dwExtraInfo = g.mouseinfo.marker.load(std::memory_order_relaxed);

	SendInput(1, &input, sizeof(INPUT));
}

//void pressMouse1(bool press) {
//	if (press) {
//		ms.press(VK_LBUTTON);
//	}
//	else {
//		ms.release();
//	}
//}

void Control::driveMouse() {

	static bool complete = false;
	weaponData currwpn;
	static size_t maxInstructions = 0;
	static int cycles = 0;

	static float xAccumulator = 0;
	static float yAccumulator = 0;

	// New variables for smoothin
	static int currentIteration = 0;
	static float totalCorrectionX = 0;
	static float totalCorrectionY = 0;
	static float smoothedCorrectionX = 0;
	static float smoothedCorrectionY = 0;

	while (!g.shutdown) {
		// Check if the selected weapon has changed
		CHI.mutex_.lock();
		if (!(currwpn == CHI.activeWeapon)) {
			currwpn = CHI.activeWeapon;
			CHI.mutex_.unlock();

			// Print the new weapon data
			/*for (auto const& data : currwpn.values) {
				std::cout << "{" << data[0] << ", " << data[1] << ", " << data[2] << "}," << std::endl;
			}*/

			// Update maxInstructions
			maxInstructions = currwpn.values.size();
		}
		else {
			CHI.mutex_.unlock();
		}

		int smoothingIterations = g.aimbotinfo.smoothing;

		while ((CHI.currAutofire ? g.mouseinfo.l_mouse_down && g.mouseinfo.r_mouse_down : GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON)) && !complete && !CHI.weaponOffOverride) {
			for (int index = 0; index < maxInstructions; index++) {
				auto& instruction = currwpn.values[index];

				float x = instruction[0] * CHI.activeWeaponSensXModifier;
				float y = instruction[1] * CHI.activeWeaponSensYModifier;
				float duration = instruction[2];

				auto currtime = std::chrono::high_resolution_clock::now();
				float int_timer = 0;
				auto nextExecution = currtime;

				while (int_timer < duration / 1000.f && (CHI.currAutofire ? g.mouseinfo.l_mouse_down && g.mouseinfo.r_mouse_down : GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON))) {
					nextExecution += std::chrono::microseconds(static_cast<long long>(10000)); // 10 milliseconds in microseconds
					auto elapsed = std::chrono::high_resolution_clock::now() - currtime;
					int_timer = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0f;

					xAccumulator += x;
					yAccumulator += y;

					int xMove = static_cast<int>(xAccumulator);
					int yMove = static_cast<int>(yAccumulator);

					xAccumulator -= xMove;
					yAccumulator -= yMove;

					// Apply smoothing
					totalCorrectionX = g.aimbotinfo.correctionX;
					totalCorrectionY = g.aimbotinfo.correctionY;
					currentIteration++;

					if (currentIteration >= smoothingIterations) {
						smoothedCorrectionX = totalCorrectionX / smoothingIterations;
						smoothedCorrectionY = totalCorrectionY / smoothingIterations;

						totalCorrectionX = 0;
						totalCorrectionY = 0;
						currentIteration = 0;
					}

					xMove += std::clamp(static_cast<int>(smoothedCorrectionX), -g.aimbotinfo.maxDistance, g.aimbotinfo.maxDistance);
					yMove += std::clamp(static_cast<int>(smoothedCorrectionY), -g.aimbotinfo.maxDistance, g.aimbotinfo.maxDistance);
					
					ms.mouse_move(0, xMove, yMove, 0);
					
					if (CHI.currAutofire && cycles >= 8 && g.mouseinfo.l_mouse_down) {
						pressMouse1(true);
						pressMouse1(false);
					}

					/*for (auto const& data : currwpn.values) {
						std::cout << "{" << x << ", " << y << ", " << duration << "}," << std::endl;
					}*/

					cycles++;

					if (CHI.potato) {
						std::this_thread::sleep_until(nextExecution);
					}
					else {
						ut.preciseSleepUntil(nextExecution);
					}
				}

				if (index == maxInstructions - 1) {
					complete = true;
				}
			}
		}
		
		if (!g.mouseinfo.l_mouse_down || !g.mouseinfo.r_mouse_down) {

			if (cycles > 8) {
				pressMouse1(false);
			}

			complete = false;
			cycles = 0;
			xAccumulator = 0;
			yAccumulator = 0;

			totalCorrectionX = 0;
			totalCorrectionY = 0;
			currentIteration = 0;
			smoothedCorrectionX = 0;
			smoothedCorrectionY = 0;
		}

		if (CHI.potato) {
			std::this_thread::sleep_for(std::chrono::nanoseconds(500));
		}
		//ut.preciseSleepFor(0.0005); // uses too much resources
	}
}