#include <windows.h>
#include <thread>

int main();


INT WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, PSTR /*lpCmdLine*/, INT /*nCmdShow*/)
{
	int exit_result = main();
	return exit_result;
}

//BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
//    if (fdwReason == DLL_PROCESS_ATTACH) {
//        std::thread appThread(main);
//        appThread.detach(); // Allows the thread to run independently; handle with care
//    }
//    return TRUE;
//}
