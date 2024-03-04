#include <windows.h>
#include <thread>

int main();
std::thread gAppThread;

#if defined _DEBUG
INT WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
	int exit_result = main();
	return exit_result;
}
#else

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        gAppThread = std::thread(main);
        //appThread.detach(); // Allows the thread to run independently; handle with care
    }
    else if (fdwReason == DLL_PROCESS_DETACH) {
        // Wait for the thread to finish
        if (gAppThread.joinable()) {
            gAppThread.join();
        }
    }
    return TRUE;
}
#endif