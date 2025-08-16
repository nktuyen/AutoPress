#pragma once
#include "afx.h"

typedef struct WindowInfo_
{
	HWND hWnd;
	TCHAR Text[255];
	DWORD ProcessId;
} WindowInfo;
