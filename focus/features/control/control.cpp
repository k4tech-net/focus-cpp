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

void controlledMouseMove(int totalX, int totalY) {
	const int MAX_MOVE = 250;
	const std::chrono::milliseconds DELAY(1);

	int remainingX = totalX;
	int remainingY = totalY;

	while (remainingX != 0 || remainingY != 0) {
		int moveX = std::clamp(remainingX, -MAX_MOVE, MAX_MOVE);
		int moveY = std::clamp(remainingY, -MAX_MOVE, MAX_MOVE);

		ms.moveR(moveX, moveY);

		remainingX -= moveX;
		remainingY -= moveY;

		std::this_thread::sleep_for(DELAY);
	}
}

void Control::driveMouse() {

	static bool complete = false;
	weaponData currwpn;
	static size_t maxInstructions = 0;
	static int cycles = 0;

	static bool flipFlop = true;

	static float xAccumulator = 0;
	static float yAccumulator = 0;

	while (!globals.shutdown) {

		// Check if the selected weapon has changed
		if (settings.activeState.weaponDataChanged) {
			settings.activeState.weaponDataChanged = false;

			// Validate character index is within bounds
			if (settings.activeState.selectedCharacterIndex >= 0 &&
				settings.activeState.selectedCharacterIndex < settings.characters.size()) {

				// Get reference to current character for readability
				const auto& character = settings.characters[settings.activeState.selectedCharacterIndex];

				// Validate that the character has weapons data
				if (!character.weapondata.empty()) {
					// Ensure weapon indices are valid
					int primaryIndex = 0;
					int secondaryIndex = 0;

					// Validate primary weapon index is within bounds
					if (character.selectedweapon.size() > 0) {
						primaryIndex = std::min(character.selectedweapon[0],
							static_cast<int>(character.weapondata.size()) - 1);
					}

					// Validate secondary weapon index is within bounds
					if (character.selectedweapon.size() > 1) {
						secondaryIndex = std::min(character.selectedweapon[1],
							static_cast<int>(character.weapondata.size()) - 1);
					}

					// Select the appropriate weapon based on active state
					if (settings.activeState.isPrimaryActive) {
						currwpn = character.weapondata[primaryIndex];
					}
					else {
						currwpn = character.weapondata[secondaryIndex];
					}

					// Update max instructions for the weapon
					maxInstructions = currwpn.values.size();
				}
				else {
					// Handle case where character has no weapons
					maxInstructions = 0;
				}
			}
			else {
				// Handle case where character index is invalid
				maxInstructions = 0;
			}
		}

		// Check mouse button states based on keymode
		bool mouseCondition = false;
		bool l_mouse = currwpn.rapidfire ? globals.mouseinfo.l_mouse_down.load() : (GetAsyncKeyState(VK_LBUTTON) != 0);
		bool r_mouse = currwpn.rapidfire ? globals.mouseinfo.r_mouse_down.load() : (GetAsyncKeyState(VK_RBUTTON) != 0);

		switch (settings.misc.recoilKeyMode) {
		case 0: // Both buttons
			mouseCondition = l_mouse && r_mouse;
			break;
		case 1: // Left button only
			mouseCondition = l_mouse;
			break;
		case 2: // Right button only
			mouseCondition = r_mouse;
			break;
		case 3: // Custom key
			mouseCondition = settings.misc.hotkeys.IsActive(HotkeyIndex::RecoilKey);
			break;
		}

		while (mouseCondition && !complete && !settings.activeState.weaponOffOverride) {
			for (int index = 0; index < maxInstructions; index++) {
				auto& instruction = currwpn.values[index];
				float x = instruction[0];
				float y = instruction[1];
				float duration = instruction[2];

				auto currtime = std::chrono::high_resolution_clock::now();
				float int_timer = 0;
				auto nextExecution = currtime;

				while (int_timer < duration / 1000.f && mouseCondition) {
					nextExecution += std::chrono::microseconds(static_cast<long long>(10000));
					auto elapsed = std::chrono::high_resolution_clock::now() - currtime;
					int_timer = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() / 1000.0f;

					std::vector<float> sens = settings.activeState.sensMultiplier;
					xAccumulator += x * sens[0];
					yAccumulator += y * sens[1];

					int xMove = static_cast<int>(xAccumulator);
					int yMove = static_cast<int>(yAccumulator);

					xAccumulator -= xMove;
					yAccumulator -= yMove;

					ms.moveR(xMove, yMove);

					if (currwpn.rapidfire && cycles >= 8 && globals.mouseinfo.l_mouse_down) {
						pressMouse1(flipFlop);
						flipFlop = !flipFlop;
					}

					cycles++;

					if (settings.globalSettings.potato) {
						std::this_thread::sleep_until(nextExecution);
					}
					else {
						ut.preciseSleepUntil(nextExecution);
					}

					// Update mouseCondition
					l_mouse = currwpn.rapidfire ? globals.mouseinfo.l_mouse_down.load() : (GetAsyncKeyState(VK_LBUTTON) != 0);
					r_mouse = currwpn.rapidfire ? globals.mouseinfo.r_mouse_down.load() : (GetAsyncKeyState(VK_RBUTTON) != 0);

					switch (settings.misc.recoilKeyMode) {
					case 0:
						mouseCondition = l_mouse && r_mouse;
						break;
					case 1:
						mouseCondition = l_mouse;
						break;
					case 2:
						mouseCondition = r_mouse;
						break;
					case 3: // Custom key
						mouseCondition = settings.misc.hotkeys.IsActive(HotkeyIndex::RecoilKey);
						break;
					}
				}

				if (index == maxInstructions - 1) {
					complete = true;
				}
			}
		}
		
		if (!mouseCondition) {

			if (cycles > 8 || !flipFlop) {
				pressMouse1(false);
				flipFlop = true;
			}

			complete = false;
			cycles = 0;
			xAccumulator = 0;
			yAccumulator = 0;

			//if (GetAsyncKeyState(VK_RSHIFT)) {
				//std::this_thread::sleep_for(std::chrono::seconds(2));

				//float oldFov = 82.0f;
				//float newFov = settings.fov;

				//float fovDifference = newFov - oldFov;
				//float quadratic_coef = 0.0000153f;  // Solved with trig :gangster:
				//float linear_coef = 0.0013043f;
				//float fovModifier = 1.0f + (quadratic_coef * fovDifference * fovDifference) +
				//	(linear_coef * fovDifference);

				//std::cout << "fovCompensation: " << fovModifier << std::endl;

				//controlledMouseMove(7274 * fovModifier, 0);

				//std::cout << 7274 * fovModifier << std::endl;

				// Siege
				// 82 = 7274
				// 60 = 7119
				// 90 = 7357

				// Rust
				// 90 = 8732
				// 70 = 11228
				// 80 = 9824

				//Overwatch
				// all = 6284
			//}
		}

		if (settings.globalSettings.potato) {
			std::this_thread::sleep_for(std::chrono::nanoseconds(500));
		}
		//ut.preciseSleepFor(0.0005); // uses too much resources
	}
}

void Control::driveAimbot() {
	// Initial PID state
	PIDController pidX(settings.aimbotData.pidSettings.proportional, settings.aimbotData.pidSettings.integral, settings.aimbotData.pidSettings.derivative);
	PIDController pidY(settings.aimbotData.pidSettings.proportional, settings.aimbotData.pidSettings.integral, settings.aimbotData.pidSettings.derivative);
	pidX.setIntegralRampTime(settings.aimbotData.pidSettings.rampUpTime);
	pidY.setIntegralRampTime(settings.aimbotData.pidSettings.rampUpTime);

	std::chrono::steady_clock::time_point lastTriggerTime = std::chrono::steady_clock::now();
	bool canTrigger = true;

	bool burstActive = false;
	std::chrono::steady_clock::time_point burstStartTime;

	while (!globals.shutdown) {
		if (settings.activeState.pidDataChanged) {
			pidX.setTunings(settings.aimbotData.pidSettings.proportional, settings.aimbotData.pidSettings.integral, settings.aimbotData.pidSettings.derivative);
			pidY.setTunings(settings.aimbotData.pidSettings.proportional, settings.aimbotData.pidSettings.integral, settings.aimbotData.pidSettings.derivative);
			pidX.setIntegralRampTime(settings.aimbotData.pidSettings.rampUpTime);
			pidY.setIntegralRampTime(settings.aimbotData.pidSettings.rampUpTime);
			settings.activeState.pidDataChanged = false;
		}

		// Check activation conditions based on keymode
		bool aimbotCondition = false;
		bool triggerBotCondition = false;
		bool l_mouse = globals.mouseinfo.l_mouse_down.load();
		bool r_mouse = globals.mouseinfo.r_mouse_down.load();

		switch (settings.misc.aimKeyMode) {
		case 0: // Both buttons
			aimbotCondition = l_mouse && r_mouse;
			break;
		case 1: // Left button only
			aimbotCondition = l_mouse;
			break;
		case 2: // Right button only
			aimbotCondition = r_mouse;
			break;
		case 3: // Custom key
			aimbotCondition = settings.misc.hotkeys.IsActive(HotkeyIndex::AimKey);
			break;
		}

		triggerBotCondition = settings.misc.hotkeys.IsActive(HotkeyIndex::TriggerKey);
		if (!triggerBotCondition) {
			canTrigger = true;

			// End burst if triggerbot key is released
			if (burstActive) {
				pressMouse1(false);
				burstActive = false;
			}
		}

		// Main aimbot loop
		while ((aimbotCondition || triggerBotCondition) && !settings.activeState.weaponOffOverride) {
			// Calculate PID corrections
			float pidCorrectionX = 0;
			float pidCorrectionY = 0;

			// Check if burst is active and should be terminated
			auto currentTime = std::chrono::steady_clock::now();
			if (burstActive) {
				auto burstElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - burstStartTime).count();
				if (burstElapsed >= settings.aimbotData.triggerBurstDuration) {
					// Only release if we initiated the press (don't interfere with user holding mouse1)
					if (!l_mouse) {
						pressMouse1(false);
					}
					burstActive = false;
				}
			}

			// Check if enough time has passed since last trigger
			auto timeSinceLastTrigger = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTriggerTime).count();
			if (timeSinceLastTrigger >= settings.aimbotData.triggerSleep) {
				canTrigger = true;
			}

			// Only calculate corrections if we have valid tracking data
			if (settings.aimbotData.correctionX != 0 || settings.aimbotData.correctionY != 0) {

				// Triggerbot FOV Check
				if (triggerBotCondition && canTrigger) {
					const float triggerbotFovPercentage = settings.aimbotData.triggerFov / 50.f;
					const int triggerbotFovPixelsX = static_cast<int>(triggerbotFovPercentage * globals.capture.desktopWidth);
					const int triggerbotFovPixelsY = static_cast<int>(triggerbotFovPercentage * globals.capture.desktopHeight);
					bool withinTriggerbotFov = std::abs(settings.aimbotData.correctionX) < triggerbotFovPixelsX && std::abs(settings.aimbotData.correctionY) < triggerbotFovPixelsY;

					if (withinTriggerbotFov) {
						// Check if mouse1 is already being held down
						bool mouseAlreadyDown = globals.mouseinfo.l_mouse_down.load(std::memory_order_relaxed);

						// Handle burst mode vs. single click mode
						if (settings.aimbotData.triggerBurstDuration > 0) {
							// Burst mode
							if (!mouseAlreadyDown && !burstActive) {
								// Start burst only if user isn't already holding mouse1 and no burst is active
								pressMouse1(true);
								burstActive = true;
								burstStartTime = currentTime;
							}
						}
						else {
							// Single click mode
							if (!mouseAlreadyDown) {
								// Only do a click if the user isn't already holding the button
								pressMouse1(true);
								std::this_thread::sleep_for(std::chrono::milliseconds(1));
								pressMouse1(false);
							}
						}

						lastTriggerTime = currentTime;
						canTrigger = false;
					}
				}

				// Aimbot FOV Check
				if (aimbotCondition) {
					const float aimbotFovPercentage = settings.aimbotData.aimFov / 50.f;
					const int aimbotFovPixelsX = static_cast<int>(aimbotFovPercentage * globals.capture.desktopWidth);
					const int aimbotFovPixelsY = static_cast<int>(aimbotFovPercentage * globals.capture.desktopHeight);
					bool withinAimbotFov = std::abs(settings.aimbotData.correctionX) < aimbotFovPixelsX && std::abs(settings.aimbotData.correctionY) < aimbotFovPixelsY;

					if (withinAimbotFov) {
						// Calculate PID corrections only if within FOV
						pidCorrectionX = pidX.calculate(settings.aimbotData.correctionX, 0);

						pidCorrectionY = pidY.calculate(settings.aimbotData.correctionY, 0) * PERCENT(settings.aimbotData.verticalCorrection);
					}
					else {
						// Outside FOV - reset corrections
						settings.aimbotData.correctionX = 0;
						settings.aimbotData.correctionY = 0;
					}
				}
			}

			int xMove = static_cast<int>(std::round(pidCorrectionX));
			int yMove = static_cast<int>(std::round(pidCorrectionY));

			xMove = std::clamp(xMove, -settings.aimbotData.maxDistance, settings.aimbotData.maxDistance);
			yMove = std::clamp(yMove, -settings.aimbotData.maxDistance, settings.aimbotData.maxDistance);

			if (xMove != 0 || yMove != 0) {
				ms.moveR(xMove, yMove);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));

			// Update mouseCondition
			l_mouse = globals.mouseinfo.l_mouse_down.load();
			r_mouse = globals.mouseinfo.r_mouse_down.load();

			switch (settings.misc.aimKeyMode) {
			case 0:
				aimbotCondition = l_mouse && r_mouse;
				break;
			case 1:
				aimbotCondition = l_mouse;
				break;
			case 2:
				aimbotCondition = r_mouse;
				break;
			case 3:
				aimbotCondition = settings.misc.hotkeys.IsActive(HotkeyIndex::AimKey);
				break;
			}

			triggerBotCondition = settings.misc.hotkeys.IsActive(HotkeyIndex::TriggerKey);
			if (!triggerBotCondition) {
				canTrigger = true;

				// End burst if triggerbot key is released
				if (burstActive) {
					pressMouse1(false);
					burstActive = false;
				}
			}
		}

		// Reset PID controllers when not active
		if (!aimbotCondition) {
			pidX.reset();
			pidY.reset();
		}

		// Ensure burst is canceled if we exit the aimbot loop
		if (burstActive && !triggerBotCondition) {
			pressMouse1(false);
			burstActive = false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void pressAndReleaseKey(KeyboardKey key, int pressDuration = 50) {
	kb.keyboard_press(key);
	std::this_thread::sleep_for(std::chrono::microseconds(pressDuration));
	kb.keyboard_release();
}

void resetLean() {
	KeyboardKey leftLeanKey = Keyboard::VKToKeyboardKey(settings.misc.hotkeys.GetHotkeyVK(HotkeyIndex::LeanLeftKey));
	KeyboardKey rightLeanKey = Keyboard::VKToKeyboardKey(settings.misc.hotkeys.GetHotkeyVK(HotkeyIndex::LeanRightKey));

	pressAndReleaseKey(leftLeanKey, 20);

	std::this_thread::sleep_for(std::chrono::microseconds(20));

	pressAndReleaseKey(rightLeanKey, 20);

	std::this_thread::sleep_for(std::chrono::microseconds(20));

	pressAndReleaseKey(rightLeanKey, 20);

	std::this_thread::sleep_for(std::chrono::microseconds(20));
}

void Control::driveKeyboard() {
	static bool moved = false;
	static int direction = 0;
	static bool init = true;

	//NetworkBlocker nb;

	//nb.initForR6();

	while (!globals.shutdown) {
		if (settings.misc.hotkeys.IsActive(HotkeyIndex::AutoQuickPeek)) {
			KeyboardKey leftLeanKey = Keyboard::VKToKeyboardKey(settings.misc.hotkeys.GetHotkeyVK(HotkeyIndex::LeanLeftKey));
			KeyboardKey rightLeanKey = Keyboard::VKToKeyboardKey(settings.misc.hotkeys.GetHotkeyVK(HotkeyIndex::LeanRightKey));

			if (init) {
				// Going Left
				if (GetAsyncKeyState(0x41) && !GetAsyncKeyState(0x44)) {
					resetLean();
					direction = 1;
					init = false;

					pressAndReleaseKey(leftLeanKey);

					std::this_thread::sleep_for(std::chrono::microseconds(50));
				}
				else if (GetAsyncKeyState(0x44) && !GetAsyncKeyState(0x41)) { // Going Right
					resetLean();
					direction = 2;
					init = false;

					pressAndReleaseKey(rightLeanKey);

					std::this_thread::sleep_for(std::chrono::microseconds(50));
				}
				else {
					moved = false;
					direction = 0;
					init = true;
				}
			}

			// Was going left, now right
			if (direction == 1 && GetAsyncKeyState(0x44) && !moved) {
				kb.keyboard_press(rightLeanKey);
				std::this_thread::sleep_for(std::chrono::milliseconds(settings.misc.quickPeekDelay));
				kb.keyboard_press(leftLeanKey);
				std::this_thread::sleep_for(std::chrono::microseconds(10));
				kb.keyboard_release();

				moved = true;
			}

			// Wait until moving left again
			if (direction == 1 && GetAsyncKeyState(0x41) && moved) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				moved = false;
			}

			// Was going right, now left
			if (direction == 2 && GetAsyncKeyState(0x41) && !moved) {
				kb.keyboard_press(leftLeanKey);
				std::this_thread::sleep_for(std::chrono::milliseconds(settings.misc.quickPeekDelay));
				kb.keyboard_press(rightLeanKey);
				std::this_thread::sleep_for(std::chrono::microseconds(10));
				kb.keyboard_release();

				moved = true;
			}

			// Wait until moving right again
			if (direction == 2 && GetAsyncKeyState(0x44) && moved) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				moved = false;
			}
		}
		else if (settings.misc.hotkeys.IsActive(HotkeyIndex::AutoHashomPeek)) {
			KeyboardKey proneKey = Keyboard::VKToKeyboardKey(settings.misc.hotkeys.GetHotkeyVK(HotkeyIndex::ProneKey));
			KeyboardKey leftLeanKey = Keyboard::VKToKeyboardKey(settings.misc.hotkeys.GetHotkeyVK(HotkeyIndex::LeanLeftKey));
			KeyboardKey rightLeanKey = Keyboard::VKToKeyboardKey(settings.misc.hotkeys.GetHotkeyVK(HotkeyIndex::LeanRightKey));

			if (init) {
				// Going Left
				if (GetAsyncKeyState(0x41) && !GetAsyncKeyState(0x44)) {
					resetLean();
					direction = 1;
					init = false;

					pressAndReleaseKey(leftLeanKey);

					std::this_thread::sleep_for(std::chrono::microseconds(50));
				}
				else if (GetAsyncKeyState(0x44) && !GetAsyncKeyState(0x41)) { // Going Right
					resetLean();
					direction = 2;
					init = false;

					pressAndReleaseKey(rightLeanKey);

					std::this_thread::sleep_for(std::chrono::microseconds(50));
				}
				else {
					moved = false;
					direction = 0;
					init = true;
				}
			}

			// Was going left, now right
			if (direction == 1 && GetAsyncKeyState(0x44) && !moved) {
				pressAndReleaseKey(proneKey);

				std::this_thread::sleep_for(std::chrono::milliseconds(settings.misc.quickPeekDelay));

				pressAndReleaseKey(proneKey);

				moved = true;
			}

			// Wait until moving left again
			if (direction == 1 && GetAsyncKeyState(0x41) && moved) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				moved = false;
			}

			// Was going right, now left
			if (direction == 2 && GetAsyncKeyState(0x41) && !moved) {
				pressAndReleaseKey(proneKey);

				std::this_thread::sleep_for(std::chrono::milliseconds(settings.misc.quickPeekDelay));

				pressAndReleaseKey(proneKey);

				moved = true;
			}

			// Wait until moving right again
			if (direction == 2 && GetAsyncKeyState(0x44) && moved) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				moved = false;
			}
		}
		else {
			moved = false;
			direction = 0;
			init = true;
		}

		if (settings.misc.hotkeys.IsActive(HotkeyIndex::FakeSpinBot)) {
			
			ms.moveR(50000, 1000);
		}

		/*if (settings.misc.hotkeys.IsActive(HotkeyIndex::DimXKey)) {
			if (!nb.isCurrentlyBlocking()) {
				nb.enableBlocking();
			}
		}
		else if (nb.isCurrentlyBlocking()) {
			nb.disableBlocking();
		}*/

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}