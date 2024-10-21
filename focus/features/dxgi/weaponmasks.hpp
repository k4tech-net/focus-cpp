#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <xorstr.hpp>
#include <bitset>

struct WeaponMask {
    std::string name;
    std::vector<uchar> mask;
    cv::Size size;
};

struct ProcessedMask {
    std::string name;
    cv::Mat edges;
};

static const int HASH_SIZE = 512;
using IconHash = std::bitset<HASH_SIZE>;

extern IconHash;
extern std::vector<WeaponMask> rustMasks;
extern std::unordered_map<IconHash, std::string> operatorHashes;