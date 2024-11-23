#pragma once
#include <windows.h>
#include <string>
#include <netfw.h>
#include <chrono>
#include <shlobj.h>
#include <filesystem>

class NetworkBlocker {
private:
    std::wstring programPath;
    std::wstring ruleNameOut;
    std::wstring ruleNameIn;
    bool isBlocking = false;
    bool initialized = false;

    // COM interfaces
    INetFwPolicy2* pPolicy = nullptr;
    INetFwRules* pRules = nullptr;

    std::wstring getR6Path() {
        wchar_t localAppData[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppData))) {
            std::filesystem::path gamePath = std::filesystem::path(localAppData) /
                L"Ubisoft" / L"r6siege" / L"RainbowSix.exe";

            if (std::filesystem::exists(gamePath)) {
                return gamePath.wstring();
            }
        }
        return L"";
    }

    bool initializeCOM();
    void cleanupCOM();
    bool createRule(const std::wstring& ruleName, NET_FW_RULE_DIRECTION direction);
    void removeRule(const std::wstring& ruleName);
    bool verifyRule(const std::wstring& ruleName);

public:
    NetworkBlocker() = default;
    ~NetworkBlocker();

    bool initForR6() {
        std::wstring path = getR6Path();
        if (!path.empty()) {
            init(path);
            return true;
        }
        return false;
    }

    void init(const std::wstring& targetProgram);
    bool enableBlocking();
    bool disableBlocking();
    bool isCurrentlyBlocking() const { return isBlocking; }
};