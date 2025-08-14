
// AutoPress.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CAutoPressApp:
// See AutoPress.cpp for the implementation of this class
//

class CAutoPressApp : public CWinApp
{
public:
	CAutoPressApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CAutoPressApp theApp;