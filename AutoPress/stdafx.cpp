
// stdafx.cpp : source file that includes just the standard includes
// AutoPress.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"


int AFXAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, int nCmdShow)
{
	ASSERT(hPrevInstance == NULL);

	int nReturnCode = -1;
	CWinThread* pThread = AfxGetThread();
	CWinApp* pApp = AfxGetApp();

	// AFX internal initialization
	if (!AfxWinInit(hInstance, hPrevInstance, (LPTSTR)lpCmdLine, nCmdShow))
		goto InitFailure;

	// App global initializations (rare)
	if (pApp != NULL && !pApp->InitApplication())
		goto InitFailure;

	// Perform specific initializations
	if (!pThread->InitInstance())
	{
		if (pThread->m_pMainWnd != NULL)
		{
			TRACE(traceAppMsg, 0, "Warning: Destroying non-NULL m_pMainWnd\n");
			pThread->m_pMainWnd->DestroyWindow();
		}
		nReturnCode = pThread->ExitInstance();
		goto InitFailure;
	}
	nReturnCode = pThread->Run();

InitFailure:
#ifdef _DEBUG
	// Check for missing AfxLockTempMap calls
	if (AfxGetModuleThreadState()->m_nTempMapLock != 0)
	{
		TRACE(traceAppMsg, 0, "Warning: Temp map lock count non-zero (%ld).\n",
			AfxGetModuleThreadState()->m_nTempMapLock);
	}
	AfxLockTempMaps();
	AfxUnlockTempMaps(-1);
#endif

	AfxWinTerm();
	return nReturnCode;
}