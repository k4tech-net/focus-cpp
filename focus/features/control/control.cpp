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
	input.mi.dwExtraInfo = globals.mouseinfo.marker.load(std::memory_order_relaxed);

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

	static bool flipFlop = true;

	static float xAccumulator = 0;
	static float yAccumulator = 0;

	// New variables for smoothin
	static int currentIteration = 0;
	static float totalCorrectionX = 0;
	static float totalCorrectionY = 0;
	static float smoothedCorrectionX = 0;
	static float smoothedCorrectionY = 0;

	while (!globals.shutdown) {
		// Check if the selected weapon has changed

		if (settings.weaponDataChanged) {
			settings.weaponDataChanged = false;
			
			if (settings.isPrimaryActive) {
				currwpn = settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[0]];
			}
			else {
				currwpn = settings.characters[settings.selectedCharacterIndex].weapondata[settings.characters[settings.selectedCharacterIndex].selectedweapon[1]];
			}

			maxInstructions = currwpn.values.size();
		}

		int smoothingIterations = settings.aimbotData.smoothing;

		while ((currwpn.autofire ? globals.mouseinfo.l_mouse_down && globals.mouseinfo.r_mouse_down : GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON)) && !complete && !settings.weaponOffOverride) {
			for (int index = 0; index < maxInstructions; index++) {
				auto& instruction = currwpn.values[index];

				float x = instruction[0];
				float y = instruction[1];
				float duration = instruction[2];

				auto currtime = std::chrono::high_resolution_clock::now();
				float int_timer = 0;
				auto nextExecution = currtime;

				while (int_timer < duration / 1000.f && (currwpn.autofire ? globals.mouseinfo.l_mouse_down && globals.mouseinfo.r_mouse_down : GetAsyncKeyState(VK_LBUTTON) && GetAsyncKeyState(VK_RBUTTON))) {
					nextExecution += std::chrono::microseconds(static_cast<long long>(10000)); // 10 milliseconds in microseconds
					auto elapsed = std::chrono::high_resolution_clock::now() - currtime;
					int_timer = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0f;

					std::vector<float> sens = settings.sensMultiplier;

					xAccumulator += x * sens[0];
					yAccumulator += y * sens[1];

					int xMove = static_cast<int>(xAccumulator);
					int yMove = static_cast<int>(yAccumulator);

					xAccumulator -= xMove;
					yAccumulator -= yMove;

					// Apply smoothing
					totalCorrectionX = settings.aimbotData.correctionX;
					totalCorrectionY = settings.aimbotData.correctionY;
					currentIteration++;

					if (currentIteration >= smoothingIterations) {
						smoothedCorrectionX = totalCorrectionX / smoothingIterations;
						smoothedCorrectionY = totalCorrectionY / smoothingIterations;

						totalCorrectionX = 0;
						totalCorrectionY = 0;
						currentIteration = 0;
					}

					xMove += std::clamp(static_cast<int>(smoothedCorrectionX), -settings.aimbotData.maxDistance, settings.aimbotData.maxDistance);
					yMove += std::clamp(static_cast<int>(smoothedCorrectionY), -settings.aimbotData.maxDistance, settings.aimbotData.maxDistance);
					
					ms.moveR(xMove, yMove);
			
					if (currwpn.autofire && cycles >= 8 && globals.mouseinfo.l_mouse_down) {
						pressMouse1(flipFlop);
						flipFlop = !flipFlop;
					}

					//std::cout << "X: " << xMove << " Y: " << yMove << " Cycles: " << cycles << std::endl;

					cycles++;

					if (settings.potato) {
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
		
		if (!globals.mouseinfo.l_mouse_down || !globals.mouseinfo.r_mouse_down) {

			if (cycles > 8 || !flipFlop) {
				pressMouse1(false);
				flipFlop = true;
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

		if (settings.potato) {
			std::this_thread::sleep_for(std::chrono::nanoseconds(500));
		}
		//ut.preciseSleepFor(0.0005); // uses too much resources
	}
}