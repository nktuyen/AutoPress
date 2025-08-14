#include <afxwin.h>

typedef struct KeyBdEvent_
{
	HWND hWnd;
	DWORD DelayTime;
	BYTE  VirtualKeyCode;
	BYTE VirtualScanCode;
	BOOL Extended;
	BOOL KeyUp;
} KeyBdEvent;