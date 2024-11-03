#pragma once

#include <math.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")

#include <xorstr.hpp>

typedef int BOOL;

class Mouse {
public:
	BOOL mouse_open(void);
	void mouse_close(void);
	void mouse_move(char button, char x, char y, char wheel);
	void moveR(int x, int y);
	void press(char button);
	void release();
	void scroll(char wheel);

	std::wstring findDriver();
};