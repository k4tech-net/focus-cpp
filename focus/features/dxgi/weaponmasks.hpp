#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <xorstr.hpp>

struct WeaponMask {
    std::string name;
    std::vector<uchar> mask;
    cv::Size size;
};

struct ProcessedMask {
    std::string name;
    cv::Mat edges;
};

extern std::vector<WeaponMask> rustMasks;
extern std::unordered_map<std::string, std::string> operatorHashes;