#pragma once
#include "afxwin.h"
#include <mutex>

#define KBD_PLAY_EVENT_NONE		0
#define KBD_PLAY_EVENT_INIT		1
#define KBD_PLAY_EVENT_DEINIT	2
#define KBD_PLAY_EVENT_PLAYING	3
#define KBD_PLAY_EVENT_SLEEPING	4

class CKeysPlayThread : public CWinThread
{
public:
	CKeysPlayThread(CPtrArray* pArrKeyEvents, CWnd* pWnd = NULL, UINT nCommunicationMessageId = 0U, int nRepeatTimes = 0);
	virtual ~CKeysPlayThread(void);
	BOOL IsRunning();
	void Stop();
protected:
	void SetRunning(BOOL bRun);
	BOOL InitInstance() override;
	int ExitInstance() override;
private:
	int Run() override;
private:
	BOOL m_bRunning;
	CWnd* m_pWnd;
	UINT m_nCommnMessageId;
	CPtrArray* m_pArrKeyEvents;
	CMutex m_mutex;
	int m_iRepeatTimes;
};

