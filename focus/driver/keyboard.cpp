#include "keyboard.hpp"

static HANDLE g_input;
static IO_STATUS_BLOCK g_io;

BOOL g_found_keyboard;

BOOL Keyboard::callKeyboard(KEYBOARD_IO* buffer)
{
    IO_STATUS_BLOCK block;
    return NtDeviceIoControlFile(g_input, 0, 0, 0, &block, 0x2A200C, buffer, sizeof(KEYBOARD_IO), 0, 0) == 0L;
}

static NTSTATUS device_initialize(PCWSTR device_name)
{
    UNICODE_STRING name;
    OBJECT_ATTRIBUTES attr;

    RtlInitUnicodeString(&name, device_name);
    InitializeObjectAttributes(&attr, &name, 0, NULL, NULL);

    NTSTATUS status = NtCreateFile(&g_input, GENERIC_WRITE | SYNCHRONIZE, &attr, &g_io, 0,
        FILE_ATTRIBUTE_NORMAL, 0, 3, FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT, 0, 0);

    return status;
}

std::vector<std::wstring> parseStringKB(const std::wstring& inputString) {
    std::vector<std::wstring> parts;
    std::wstringstream ss(inputString);
    std::wstring part;

    while (std::getline(ss, part, L'\\')) {
        parts.push_back(part);
    }

    return parts;
}

BOOL Keyboard::keyboard_open()
{
    NTSTATUS status = 0;

    if (g_input == 0) {
        std::wstring driver = findDriver();

        if (driver != L"") {
            std::vector parsedDriver = parseStringKB(driver);

            wchar_t buffer0[256];
            swprintf(buffer0, 256, L"\\??\\%s#%s#%s#{1abc05c0-c378-41b9-9cef-df1aba82b015}",
                parsedDriver[0].c_str(), parsedDriver[1].c_str(), parsedDriver[2].c_str());

            status = device_initialize(buffer0);

            if (NT_SUCCESS(status)) {
                g_found_keyboard = 1;
                return true;
            }
            else {
                g_found_keyboard = 0;
                return false;
            }
        }
        else {
            // Driver not found
            return false;
        }
    }
    return g_input != 0;  // Return true if g_input is already initialized
}

void Keyboard::keyboard_close()
{
    if (g_input != 0) {
        NtClose(g_input);  //ZwClose
        g_input = 0;
    }
}

BOOL Keyboard::keyboard_press(KeyboardKey key)
{
    return sendKeyboardInput(key);
}

BOOL Keyboard::keyboard_release()
{
    return sendKeyboardInput();
}

BOOL Keyboard::keyboard_type(const std::vector<KeyboardKey>& keys)
{
    for (const auto& key : keys) {
        if (!keyboard_press(key) || !keyboard_release()) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL Keyboard::sendKeyboardInput(KeyboardKey b0, KeyboardKey b1, KeyboardKey b2, KeyboardKey b3, KeyboardKey b4, KeyboardKey b5)
{
    KEYBOARD_IO io = { 0, 0, b0, b1, b2, b3, b4, b5 };
    if (!callKeyboard(&io)) {
        keyboard_close();
        keyboard_open();
        std::cout << xorstr_("Driver Error: ") << GetLastError() << std::endl;
        return FALSE;
    }
    return TRUE;
}

BOOL Keyboard::keyboard_multi_press(const std::vector<KeyboardKey>& keys)
{
    if (keys.size() > 6) {
        std::cout << xorstr_("Error: Can't press more than 6 keys simultaneously") << std::endl;
        return FALSE;
    }

    KeyboardKey b0 = keys.size() > 0 ? keys[0] : KeyboardKey{};
    KeyboardKey b1 = keys.size() > 1 ? keys[1] : KeyboardKey{};
    KeyboardKey b2 = keys.size() > 2 ? keys[2] : KeyboardKey{};
    KeyboardKey b3 = keys.size() > 3 ? keys[3] : KeyboardKey{};
    KeyboardKey b4 = keys.size() > 4 ? keys[4] : KeyboardKey{};
    KeyboardKey b5 = keys.size() > 5 ? keys[5] : KeyboardKey{};

    return sendKeyboardInput(b0, b1, b2, b3, b4, b5);
}

BOOL Keyboard::keyboard_release_all()
{
    KEYBOARD_IO io = { 0 };
    return callKeyboard(&io);
}

std::wstring Keyboard::findDriver() {
    HKEY hKey;
    std::wstring deviceKeyName = L"SYSTEM\\ControlSet001\\Control\\DeviceClasses\\{dfbedcdb-2148-416d-9e4d-cecc2424128c}";
    std::wstring subkeyName = L"";
    std::wstring deviceLocation;

    // Open the registry key for the device classes
    LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, deviceKeyName.c_str(), 0, KEY_READ, &hKey);
    if (result == ERROR_SUCCESS) {
        // Enumerate subkeys to find the one containing DeviceInstance
        DWORD index = 0;
        WCHAR subkeyNameBuffer[MAX_PATH];
        DWORD subkeyNameSize = MAX_PATH;
        while (RegEnumKeyEx(hKey, index, subkeyNameBuffer, &subkeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            subkeyName = subkeyNameBuffer;
            HKEY hSubKey;
            // Open the subkey
            result = RegOpenKeyEx(hKey, subkeyName.c_str(), 0, KEY_READ, &hSubKey);
            if (result == ERROR_SUCCESS) {
                // Check if DeviceInstance entry exists
                DWORD valueType;
                WCHAR deviceInstanceBuffer[MAX_PATH];
                DWORD deviceInstanceSize = MAX_PATH;
                result = RegQueryValueEx(hSubKey, L"DeviceInstance", NULL, &valueType, reinterpret_cast<LPBYTE>(deviceInstanceBuffer), &deviceInstanceSize);
                if (result == ERROR_SUCCESS && valueType == REG_SZ) {
                    deviceLocation = deviceInstanceBuffer;
                    RegCloseKey(hSubKey);  // Close the subkey
                    break;  // Exit loop since DeviceInstance entry is found
                }
                RegCloseKey(hSubKey);  // Close the subkey
            }
            ++index;
            subkeyNameSize = MAX_PATH;
        }

        // Close the registry key
        RegCloseKey(hKey);
        return deviceLocation;
    }
    else {
        return deviceLocation;
    }
}