#include "mouse.hpp"

typedef struct {
	char button;
	char x;
	char y;
	char wheel;
	char unk1;
} MOUSE_IO;

#define MOUSE_PRESS 1
#define MOUSE_RELEASE 2
#define MOUSE_MOVE 3
#define MOUSE_CLICK 4

static HANDLE g_input;
static IO_STATUS_BLOCK g_io;

BOOL g_found_mouse;

static BOOL callmouse(MOUSE_IO* buffer)
{
	IO_STATUS_BLOCK block;
	return NtDeviceIoControlFile(g_input, 0, 0, 0, &block, 0x2a2010, buffer, sizeof(MOUSE_IO), 0, 0) == 0L;
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

std::vector<std::wstring> parseString(const std::wstring& inputString) {
	std::vector<std::wstring> parts;
	std::wstringstream ss(inputString);
	std::wstring part;

	while (std::getline(ss, part, L'\\')) {
		parts.push_back(part);
	}

	return parts;
}

BOOL Mouse::mouse_open()
{
	NTSTATUS status = 0;

	if (g_input == 0) {
		std::wstring driver = findDriver();

		if (driver != L"") {
			std::vector parsedDriver = parseString(driver);

			wchar_t buffer0[256];
			swprintf(buffer0, 256, L"\\??\\%s#%s#%s#{1abc05c0-c378-41b9-9cef-df1aba82b015}",
				parsedDriver[0].c_str(), parsedDriver[1].c_str(), parsedDriver[2].c_str());

			status = device_initialize(buffer0);

			if (NT_SUCCESS(status)) {
				g_found_mouse = 1;
				return true;
			}
			else {
				g_found_mouse = 0;
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

void Mouse::mouse_close()
{
	if (g_input != 0) {
		NtClose(g_input);  //ZwClose
		g_input = 0;
	}
}

void Mouse::mouse_move(char button, char x, char y, char wheel)
{
	MOUSE_IO io;
	io.unk1 = 0;
	io.button = button;
	io.x = x;
	io.y = y;
	io.wheel = wheel;

	if (!callmouse(&io)) {
		mouse_close();
		mouse_open();
		std::cout << "Driver Error: " << GetLastError() << std::endl;
	}
}

void Mouse::moveR(int x, int y)
{
	if (abs(x) > 127 || abs(y) > 127) {
		int x_left = x; int y_left = y;
		if (abs(x) > 127) {
			mouse_move(0, int(x / abs(x)) * 127, 0, 0);
			x_left = x - int(x / abs(x)) * 127;
		}
		else { mouse_move(0, int(x), 0, 0); x_left = 0; }

		if (abs(y) > 127) {
			mouse_move(0, 0, int(y / abs(y)) * 127, 0);
			y_left = y - int(y / abs(y)) * 127;
		}
		else { mouse_move(0, 0, int(y), 0); y_left = 0; }

		return moveR(x_left, y_left);
	}
	else { mouse_move(0, x, y, 0); }
}

void Mouse::press(char button)
{
	mouse_move(button, 0, 0, 0);
}

void Mouse::release()
{
	mouse_move(0, 0, 0, 0);
}

void Mouse::scroll(char wheel)
{
	mouse_move(0, 0, 0, -wheel);
}

std::wstring Mouse::findDriver() {
	HKEY hKey;
	std::wstring deviceKeyName = L"SYSTEM\\ControlSet001\\Control\\DeviceClasses\\{1abc05c0-c378-41b9-9cef-df1aba82b015}";
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
					//std::wcout << "Location: " << deviceLocation << std::endl;
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
		//std::cerr << "Failed to open registry key for the device classes. Error code: " << result << std::endl;
		return deviceLocation;
	}
}