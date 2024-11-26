#pragma once

#include "../../includes.hpp"

class Control
{
public:
	void driveMouse();
	void driveKeyboard();
};

class PIDController {
private:
    // Base PID coefficients
    float baseKp, Ki, Kd;
    float maxKp;  // Maximum P value for large errors

    // Error tracking
    float previousError;
    float integralError;
    float lastCorrection;

    // Timing
    std::chrono::steady_clock::time_point lastUpdateTime;

    // Integral windup prevention
    float integralLimit;

    // Smoothing for derivative term
    float lastDerivative;
    float derivativeSmoothingFactor;

    float clamp(float value, float min, float max) {
        return std::max(min, std::min(value, max));
    }

    float calculateAdaptiveKp(float error) {
        // Increase Kp based on error magnitude
        float errorThreshold = 10.0f;  // Adjust based on your needs
        float errorRatio = std::min(std::abs(error) / errorThreshold, 1.0f);

        // Smooth transition between base and max Kp
        return baseKp + (maxKp - baseKp) * errorRatio * errorRatio;
    }

public:
    PIDController(float p = 0.03f, float i = 0.2f, float d = 0.001f)
        : baseKp(p), Ki(i), Kd(d),
        maxKp(p * 3.0f),  // Max Kp is 3x the base value
        previousError(0.0f),
        integralError(0.0f),
        lastCorrection(0.0f),
        lastDerivative(0.0f),
        derivativeSmoothingFactor(0.7f), // Adjust between 0 and 1
        integralLimit(20.0f) {
        lastUpdateTime = std::chrono::steady_clock::now();
    }

    void reset() {
        previousError = 0.0f;
        integralError = 0.0f;
        lastCorrection = 0.0f;
        lastDerivative = 0.0f;
        lastUpdateTime = std::chrono::steady_clock::now();
    }

    float calculate(float targetPosition, float currentPosition) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
        lastUpdateTime = currentTime;

        // Avoid division by zero and extreme delta times
        if (deltaTime < 0.0001f || deltaTime > 0.1f) {
            return lastCorrection;
        }

        // Calculate error
        float error = targetPosition - currentPosition;

        // Calculate adaptive Kp based on error magnitude
        float adaptiveKp = calculateAdaptiveKp(error);

        // Proportional term with adaptive Kp
        float P = adaptiveKp * error;

        // Integral term with anti-windup
        integralError = clamp(integralError + error * deltaTime, -integralLimit, integralLimit);
        float I = Ki * integralError;

        // Smoothed derivative term (on measurement to avoid derivative kick)
        float currentDerivative = (error - previousError) / deltaTime;
        float smoothedDerivative = (derivativeSmoothingFactor * lastDerivative) +
            ((1.0f - derivativeSmoothingFactor) * currentDerivative);
        float D = Kd * smoothedDerivative;

        // Store values for next iteration
        previousError = error;
        lastDerivative = smoothedDerivative;

        // Calculate final correction with additional smoothing for small movements
        float correction = P + I + D;

        // Additional smoothing for very small corrections to prevent jitter
        if (std::abs(correction) < 0.1f) {
            correction = 0.0f;
        }

        lastCorrection = correction;
        return correction;
    }

    void setTunings(float p, float i, float d) {
        baseKp = p;
        maxKp = p * 3.0f;
        Ki = i;
        Kd = d;
    }
};