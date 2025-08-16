#include "stdafx.h"
#include "KeysPlayThread.h"
#include "KbdEventStruct.h"
#include "KeyCodeMaper.h"

CKeysPlayThread::CKeysPlayThread(CPtrArray* pArrKeyEvents, CWnd* pWnd /* = NULL */, UINT nCommunicationMessageId /* = 0U */, int nRepeatTimes /* = 0 */)
	: CWinThread()
	, m_bRunning(FALSE)
	, m_pArrKeyEvents(pArrKeyEvents)
	, m_pWnd(pWnd)
	, m_nCommnMessageId(nCommunicationMessageId)
	, m_iRepeatTimes(nRepeatTimes)
{
	m_bAutoDelete = FALSE;
}

BOOL CKeysPlayThread::InitInstance()
{
	CWinThread::InitInstance();

	return TRUE;
}

int CKeysPlayThread::ExitInstance()
{
	return CWinThread::ExitInstance();
}

void CKeysPlayThread::Stop()
{
	CSingleLock locker(&m_mutex);
	m_bRunning = FALSE;
}

BOOL CKeysPlayThread::IsRunning()
{
	BOOL bRes = FALSE;
	CSingleLock locker(&m_mutex);
	bRes = m_bRunning;
	return bRes;
}

void CKeysPlayThread::SetRunning(BOOL bRun)
{
	CSingleLock locker(&m_mutex);
	m_bRunning = bRun;
}

CKeysPlayThread::~CKeysPlayThread(void)
{
}

int CKeysPlayThread::Run()
{
	SetRunning(TRUE);

	if(m_pWnd != NULL && m_nCommnMessageId != 0)
		m_pWnd->PostMessage(m_nCommnMessageId, KBD_PLAY_EVENT_INIT, NULL);

	if(m_pArrKeyEvents != NULL)
	{
		int nPlayedTimes = 0;
		KeyBdEvent *kbdEvent = nullptr;
		DWORD dwFlag = 0;
		while (m_bRunning && ((nPlayedTimes <= m_iRepeatTimes) || (m_iRepeatTimes == -1)))
		{
			for(INT_PTR i=0;i<m_pArrKeyEvents->GetCount();i++)
			{
				if(!m_bRunning)
					break;
				kbdEvent = (KeyBdEvent*)m_pArrKeyEvents->GetAt(i);
				dwFlag = 0;
				if(kbdEvent != nullptr)
				{
					dwFlag = 0;
					if(kbdEvent->Extended)
						dwFlag |= KEYEVENTF_EXTENDEDKEY;
					if(kbdEvent->KeyUp)
						dwFlag |= KEYEVENTF_KEYUP;
					if(kbdEvent->DelayTime != 0)
						Sleep(kbdEvent->DelayTime);

					if(kbdEvent->hWnd != NULL)
					{
						if(::IsWindow(kbdEvent->hWnd))
						{
							if(::GetForegroundWindow() != kbdEvent->hWnd)
								::SetForegroundWindow(kbdEvent->hWnd);
						}
						else {
							SetRunning(FALSE);
							break;
						}
					}

					if(m_pWnd != NULL && m_nCommnMessageId != 0)
						m_pWnd->PostMessage(m_nCommnMessageId, KBD_PLAY_EVENT_PLAYING, (LPARAM)kbdEvent);
					keybd_event(kbdEvent->VirtualKeyCode, kbdEvent->VirtualScanCode, dwFlag, 0);
				}
			}
			//
			nPlayedTimes++;
		}
	}

	SetRunning(FALSE);
	if(m_pWnd != NULL && m_nCommnMessageId != 0)
		m_pWnd->PostMessage(m_nCommnMessageId, KBD_PLAY_EVENT_DEINIT, NULL);
	return 0;
}