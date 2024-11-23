#include "network.hpp"
#include <comdef.h>

NetworkBlocker::~NetworkBlocker() {
    if (isBlocking) {
        disableBlocking();
    }
    cleanupCOM();
}

void NetworkBlocker::init(const std::wstring& targetProgram) {
    if (initialized) return;

    programPath = targetProgram;

    // Generate unique rule names using timestamp
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    ruleNameOut = L"BlockOut_" + std::to_wstring(now);
    ruleNameIn = L"BlockIn_" + std::to_wstring(now);

    initialized = initializeCOM();
}

bool NetworkBlocker::initializeCOM() {
    // Initialize COM
    HRESULT hr = CoInitializeEx(0, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return false;

    // Create instance of the firewall policy
    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        (void**)&pPolicy
    );
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }

    // Get the firewall rules collection
    hr = pPolicy->get_Rules(&pRules);
    if (FAILED(hr)) {
        pPolicy->Release();
        pPolicy = nullptr;
        CoUninitialize();
        return false;
    }

    return true;
}

void NetworkBlocker::cleanupCOM() {
    if (pRules) {
        pRules->Release();
        pRules = nullptr;
    }
    if (pPolicy) {
        pPolicy->Release();
        pPolicy = nullptr;
    }
    CoUninitialize();
    initialized = false;
}

bool NetworkBlocker::createRule(const std::wstring& ruleName, NET_FW_RULE_DIRECTION direction) {
    if (!initialized) return false;

    INetFwRule* pRule = nullptr;
    HRESULT hr = CoCreateInstance(
        __uuidof(NetFwRule),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwRule),
        (void**)&pRule
    );
    if (FAILED(hr)) return false;

    pRule->put_Name(_bstr_t(ruleName.c_str()));
    pRule->put_ApplicationName(_bstr_t(programPath.c_str()));
    pRule->put_Action(NET_FW_ACTION_BLOCK);
    pRule->put_Direction(direction);
    pRule->put_Enabled(VARIANT_TRUE);

    hr = pRules->Add(pRule);
    pRule->Release();

    return SUCCEEDED(hr);
}

void NetworkBlocker::removeRule(const std::wstring& ruleName) {
    if (pRules) {
        pRules->Remove(_bstr_t(ruleName.c_str()));
    }
}

bool NetworkBlocker::verifyRule(const std::wstring& ruleName) {
    if (!pRules) return false;

    INetFwRule* pRule = nullptr;
    BSTR bstrRuleName = SysAllocString(ruleName.c_str());
    HRESULT hr = pRules->Item(_bstr_t(bstrRuleName), &pRule);
    SysFreeString(bstrRuleName);

    if (SUCCEEDED(hr) && pRule) {
        pRule->Release();
        return true;
    }
    return false;
}

bool NetworkBlocker::enableBlocking() {
    if (!initialized || isBlocking) return false;

    bool success = createRule(ruleNameOut, NET_FW_RULE_DIR_OUT) &&
        createRule(ruleNameIn, NET_FW_RULE_DIR_IN);

    if (success) {
        isBlocking = true;
        return verifyRule(ruleNameOut) && verifyRule(ruleNameIn);
    }
    return false;
}

bool NetworkBlocker::disableBlocking() {
    if (!initialized || !isBlocking) return false;

    removeRule(ruleNameOut);
    removeRule(ruleNameIn);
    isBlocking = false;

    return !verifyRule(ruleNameOut) && !verifyRule(ruleNameIn);
}