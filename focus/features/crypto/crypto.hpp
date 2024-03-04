#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <openssl/aes.h>
#include <openssl/rand.h>
#include "base64.h"

#include <atomic>
#include <chrono>
#include <thread>

#pragma comment(lib, "libcrypto.lib")

class Crypto {

public:
	std::string encrypt(std::string plaintext, std::string key, std::string iv);
	std::string decrypt(std::string ciphertext, std::string key, std::string iv);
	std::string generateIV();
	std::string encryptEncode(std::string plaintext, std::string key, std::string iv);
	std::string decryptDecode(std::string ciphertext, std::string key, std::string iv);

	void verification(const char* iv, const char* verificationKey);
	void watchdog();

	std::atomic<std::chrono::steady_clock::time_point> lastAuthTime;
};