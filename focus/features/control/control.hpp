#pragma once

#include "../../includes.hpp"
#include "../network/network.hpp"

class Control
{
public:
	void driveMouse();
    void driveAimbot();
	void driveKeyboard();
};

class PIDController {
private:
    // Base PID coefficients
    float baseKp, baseKi, Kd = 0.f;
    float maxKp = 0.f;
    float maxKi = 0.f;  // Maximum integral gain

    // Error tracking
    float previousError = 0.f;
    float integralError = 0.f;
    float lastCorrection = 0.f;

    // Timing
    std::chrono::steady_clock::time_point lastUpdateTime;
    std::chrono::steady_clock::time_point startTrackingTime;
    bool isTracking = false;

    // Integral windup prevention
    float integralLimit = 0.f;

    // Integral ramp-up configuration
    float integralRampTime = 0.f;  // Time in seconds to reach full integral gain

    // Enhanced derivative filtering
    static const int DERIVATIVE_FILTER_SIZE = 5;
    std::array<float, DERIVATIVE_FILTER_SIZE> derivativeHistory;
    int derivativeHistoryIndex = 0;
    float derivativeFilterBeta = 0.f;
    float lastFilteredDerivative = 0.f;

    float clamp(float value, float min, float max) {
        return std::max(min, std::min(value, max));
    }

    float calculateAdaptiveKp(float error) {
        float errorThreshold = 10.0f;
        float errorRatio = std::min(std::abs(error) / errorThreshold, 1.0f);
        return baseKp + (maxKp - baseKp) * errorRatio * errorRatio;
    }

    float calculateAdaptiveKi(float trackingDuration) {
        if (!isTracking) return 0.0f;

        // Gradually increase Ki over the ramp time
        float rampRatio = std::min(trackingDuration / integralRampTime, 1.0f);

        // Use a smooth acceleration curve
        float smoothRatio = rampRatio * rampRatio * (3 - 2 * rampRatio);
        return baseKi * smoothRatio;
    }

    float filterDerivative(float newDerivative) {
        derivativeHistory[derivativeHistoryIndex] = newDerivative;
        derivativeHistoryIndex = (derivativeHistoryIndex + 1) % DERIVATIVE_FILTER_SIZE;

        std::array<float, DERIVATIVE_FILTER_SIZE> sortedValues = derivativeHistory;
        std::sort(sortedValues.begin(), sortedValues.end());
        float medianDerivative = sortedValues[DERIVATIVE_FILTER_SIZE / 2];

        lastFilteredDerivative = derivativeFilterBeta * lastFilteredDerivative +
            (1.0f - derivativeFilterBeta) * medianDerivative;

        float derivativeThreshold = 100.0f;
        if (std::abs(lastFilteredDerivative) > derivativeThreshold) {
            lastFilteredDerivative *= (derivativeThreshold / std::abs(lastFilteredDerivative));
        }

        return lastFilteredDerivative;
    }

public:
    PIDController(float p = 0.03f, float i = 0.2f, float d = 0.001f)
        : baseKp(p), baseKi(i), Kd(d),
        maxKp(p * 3.0f),
        previousError(0.0f),
        integralError(0.0f),
        lastCorrection(0.0f),
        lastFilteredDerivative(0.0f),
        derivativeFilterBeta(0.85f),
        integralLimit(20.0f),
        derivativeHistoryIndex(0),
        isTracking(false),
        integralRampTime(0.5f) {  // Ramp up to full I gain over 0.5 seconds
        lastUpdateTime = std::chrono::steady_clock::now();
        startTrackingTime = lastUpdateTime;
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
        isTracking = false;
    }

    float calculate(float targetPosition, float currentPosition) {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
        lastUpdateTime = currentTime;

        if (deltaTime < 0.0001f || deltaTime > 0.1f) {
            return lastCorrection;
        }

        float error = targetPosition - currentPosition;

        // Start tracking when we get non-zero error
        if (!isTracking && std::abs(error) > 0.1f) {
            isTracking = true;
            startTrackingTime = currentTime;
        }

        // Calculate tracking duration
        float trackingDuration = std::chrono::duration<float>(currentTime - startTrackingTime).count();

        float adaptiveKp = calculateAdaptiveKp(error);
        float adaptiveKi = calculateAdaptiveKi(trackingDuration);

        // Proportional term
        float P = adaptiveKp * error;

        // Integral term with adaptive gain and anti-windup
        integralError = clamp(integralError + error * deltaTime, -integralLimit, integralLimit);
        float I = adaptiveKi * integralError;

        // Enhanced derivative term
        float rawDerivative = (error - previousError) / deltaTime;
        float filteredDerivative = filterDerivative(rawDerivative);
        float D = Kd * filteredDerivative;

        previousError = error;

        // Calculate final correction with deadzone
        float correction = P + I + D;
        if (std::abs(correction) < 0.15f) {
            correction = 0.0f;
        }

        lastCorrection = correction;
        return correction;
    }

    void setTunings(float p, float i, float d) {
        baseKp = p;
        maxKp = p * 3.0f;
        baseKi = i;
        Kd = d;
    }

    void setIntegralRampTime(float seconds) {
        integralRampTime = std::max(0.1f, seconds);
    }
};