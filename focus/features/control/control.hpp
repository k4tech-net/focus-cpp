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
    float maxKp;

    // Error tracking
    float previousError;
    float integralError;
    float lastCorrection;

    // Timing
    std::chrono::steady_clock::time_point lastUpdateTime;

    // Integral windup prevention
    float integralLimit;

    // Enhanced derivative filtering
    static const int DERIVATIVE_FILTER_SIZE = 5;
    std::array<float, DERIVATIVE_FILTER_SIZE> derivativeHistory;
    int derivativeHistoryIndex;
    float derivativeFilterBeta;  // Low-pass filter coefficient
    float lastFilteredDerivative;

    float clamp(float value, float min, float max) {
        return std::max(min, std::min(value, max));
    }

    float calculateAdaptiveKp(float error) {
        float errorThreshold = 10.0f;
        float errorRatio = std::min(std::abs(error) / errorThreshold, 1.0f);
        return baseKp + (maxKp - baseKp) * errorRatio * errorRatio;
    }

    float filterDerivative(float newDerivative) {
        // Update circular buffer
        derivativeHistory[derivativeHistoryIndex] = newDerivative;
        derivativeHistoryIndex = (derivativeHistoryIndex + 1) % DERIVATIVE_FILTER_SIZE;

        // Calculate median
        std::array<float, DERIVATIVE_FILTER_SIZE> sortedValues = derivativeHistory;
        std::sort(sortedValues.begin(), sortedValues.end());
        float medianDerivative = sortedValues[DERIVATIVE_FILTER_SIZE / 2];

        // Apply exponential smoothing
        lastFilteredDerivative = derivativeFilterBeta * lastFilteredDerivative + 
                                (1.0f - derivativeFilterBeta) * medianDerivative;

        // Apply non-linear filtering for large changes
        float derivativeThreshold = 100.0f;  // Adjust based on testing
        if (std::abs(lastFilteredDerivative) > derivativeThreshold) {
            lastFilteredDerivative *= (derivativeThreshold / std::abs(lastFilteredDerivative));
        }

        return lastFilteredDerivative;
    }

public:
    PIDController(float p = 0.03f, float i = 0.2f, float d = 0.001f)
        : baseKp(p), Ki(i), Kd(d),
          maxKp(p * 3.0f),
          previousError(0.0f),
          integralError(0.0f),
          lastCorrection(0.0f),
          lastFilteredDerivative(0.0f),
          derivativeFilterBeta(0.85f),  // Increased smoothing
          integralLimit(20.0f),
          derivativeHistoryIndex(0) {
        lastUpdateTime = std::chrono::steady_clock::now();
        derivativeHistory.fill(0.0f);
    }

    void reset() {
        previousError = 0.0f;
        integralError = 0.0f;
        lastCorrection = 0.0f;
        lastFilteredDerivative = 0.0f;
        derivativeHistory.fill(0.0f);
        derivativeHistoryIndex = 0;
        lastUpdateTime = std::chrono::steady_clock::now();
    }

    float calculate(float targetPosition, float currentPosition) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
        lastUpdateTime = currentTime;

        if (deltaTime < 0.0001f || deltaTime > 0.1f) {
            return lastCorrection;
        }

        float error = targetPosition - currentPosition;
        float adaptiveKp = calculateAdaptiveKp(error);
        
        // Proportional term
        float P = adaptiveKp * error;

        // Integral term with anti-windup
        integralError = clamp(integralError + error * deltaTime, -integralLimit, integralLimit);
        float I = Ki * integralError;

        // Enhanced derivative term with sophisticated filtering
        float rawDerivative = (error - previousError) / deltaTime;
        float filteredDerivative = filterDerivative(rawDerivative);
        float D = Kd * filteredDerivative;

        previousError = error;

        // Calculate final correction with deadzone for small movements
        float correction = P + I + D;
        if (std::abs(correction) < 0.15f) {  // Slightly increased deadzone
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