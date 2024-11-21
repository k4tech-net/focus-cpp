#pragma once

#include "../../includes.hpp"

class Control
{
public:
	void driveMouse();
	void driveKeyboard();
};

class AimbotPredictor {
private:
    struct MovementState {
        float correctionX;
        float correctionY;
        std::chrono::steady_clock::time_point timestamp;
    };

    static const size_t HISTORY_SIZE = 3;
    std::deque<MovementState> history;

    float smoothedX = 0.0f;
    float smoothedY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;

    const float VELOCITY_SMOOTH = 0.7f;    // Higher = more smoothing of velocity changes
    const float SUDDEN_CHANGE_THRESHOLD = 50.0f;  // Threshold for detecting sudden direction changes

public:
    void update(float rawCorrectionX, float rawCorrectionY) {
        auto now = std::chrono::steady_clock::now();

        // Add new state to history
        history.push_front({ rawCorrectionX, rawCorrectionY, now });
        if (history.size() > HISTORY_SIZE) {
            history.pop_back();
        }

        if (history.size() >= 2) {
            // Calculate time delta
            float dt = std::chrono::duration<float>(history[0].timestamp - history[1].timestamp).count();
            if (dt > 0) {
                // Calculate instantaneous velocity
                float instVelX = (history[0].correctionX - history[1].correctionX) / dt;
                float instVelY = (history[0].correctionY - history[1].correctionY) / dt;

                // Check for sudden changes in direction
                bool suddenChange = false;
                if (history.size() >= 3) {
                    float prevVelX = (history[1].correctionX - history[2].correctionX) / dt;
                    float prevVelY = (history[1].correctionY - history[2].correctionY) / dt;

                    if (std::abs(instVelX - prevVelX) > SUDDEN_CHANGE_THRESHOLD ||
                        std::abs(instVelY - prevVelY) > SUDDEN_CHANGE_THRESHOLD) {
                        suddenChange = true;
                    }
                }

                if (suddenChange) {
                    // Reset smoothing on sudden changes
                    velocityX = instVelX;
                    velocityY = instVelY;
                    smoothedX = history[0].correctionX;
                    smoothedY = history[0].correctionY;
                }
                else {
                    // Smooth velocity changes
                    velocityX = velocityX * VELOCITY_SMOOTH + instVelX * (1.0f - VELOCITY_SMOOTH);
                    velocityY = velocityY * VELOCITY_SMOOTH + instVelY * (1.0f - VELOCITY_SMOOTH);

                    // Update smoothed positions
                    smoothedX = history[0].correctionX + velocityX * dt;
                    smoothedY = history[0].correctionY + velocityY * dt;
                }
            }
        }
        else {
            smoothedX = rawCorrectionX;
            smoothedY = rawCorrectionY;
        }
    }

    void getPredictedCorrections(float& correctionX, float& correctionY) {
        correctionX = smoothedX;
        correctionY = smoothedY;
    }

    void reset() {
        history.clear();
        smoothedX = 0.0f;
        smoothedY = 0.0f;
        velocityX = 0.0f;
        velocityY = 0.0f;
    }
};
