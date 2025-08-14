
// AutoPressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AutoPress.h"
#include "AutoPressDlg.h"
#include "afxdialogex.h"
#include "KbdEventStruct.h"
#include "KeyCodeMaper.h"
#include "KeysPlayThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAutoPressDlg dialog

LRESULT CALLBACK CAutoPressDlg::KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	return CAutoPressDlg::GetInstance()->OnKeyboardEvent(code, wParam, lParam);
}

CAutoPressDlg* CAutoPressDlg::m_pInstance = NULL;
UINT CAutoPressDlg::m_nCommunicationMessageId = 0;


CAutoPressDlg::CAutoPressDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAutoPressDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pInstance = this;
	m_bRecording = FALSE;
	m_hKbdHook = NULL;
	m_dwLastTickCount = 0;
	m_pPlayThread = NULL;
}


CAutoPressDlg::~CAutoPressDlg()
{
	ClearKeyEvents();
}

LRESULT CAutoPressDlg::OnKeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HC_ACTION && m_bRecording)
	{
		DWORD dwTickCount = GetTickCount();
		DWORD dwDiff = dwTickCount - m_dwLastTickCount;
		m_dwLastTickCount = dwTickCount;

		DWORD dwParam = (DWORD)lParam;
		DWORD wRepeatCount = LOWORD(dwParam);
		BYTE wScanCode = (BYTE) (dwParam & 0xFF0000);
		DWORD wExtendedFlag = dwParam & 0x1000000;
		DWORD wReserved = dwParam & 0x1E000000;
		DWORD wAltKeyFlag = dwParam & 0x20000000;
		DWORD wPrevKeyState = dwParam & 0x40000000;
		DWORD wKeyReleased = dwParam & 0x80000000;

		KeyBdEvent* newEvent = new KeyBdEvent;
		ZeroMemory(newEvent, sizeof(KeyBdEvent));
		newEvent->DelayTime = dwDiff;
		newEvent->VirtualKeyCode = wParam;
		newEvent->VirtualScanCode = wScanCode;
		newEvent->Extended = wExtendedFlag;
		newEvent->KeyUp = wKeyReleased;

		m_arrKeyEvents.Add(newEvent);
		CString strItem;
		CString strKeyName;
		if(!CKeyCodeMaper::Lookup(newEvent->VirtualKeyCode, strKeyName))
			strItem.Format(_T("0x%.2X %s"), newEvent->VirtualKeyCode, newEvent->KeyUp != 0 ? _T("UP") : _T("DOWN"));
		else
			strItem.Format(_T("%s %s"), strKeyName, newEvent->KeyUp != 0 ? _T("UP") : _T("DOWN"));

		int nItem = m_lstKeys.AddString(strItem);
		if(nItem != LB_ERR)
			m_lstKeys.SetItemDataPtr(nItem, newEvent);

		return 1L;
	}
	return ::CallNextHookEx(m_hKbdHook, nCode, wParam, lParam);
}


void CAutoPressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_KEYS, m_lstKeys);
	DDX_Control(pDX, IDC_BTN_RECORD, m_btnRecord);
	DDX_Control(pDX, IDC_BTN_CLEAR, m_btnClear);
	DDX_Control(pDX, IDC_BTN_PLAY, m_btnPlay);
	DDX_Control(pDX, IDC_STT_PLAYING_STATUS, m_sttPlayingStatus);
	DDX_Control(pDX, IDC_EDT_REPEAT_TIMES, m_edtRepeatTimes);
	DDX_Control(pDX, IDC_SPIN_REPEAT_TIMES, m_spinRepeatTimes);
}

BEGIN_MESSAGE_MAP(CAutoPressDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CLEAR, &CAutoPressDlg::OnBnClickedBtnClear)
	ON_BN_CLICKED(IDC_BTN_RECORD, &CAutoPressDlg::OnBnClickedBtnRecord)
	ON_BN_CLICKED(IDC_BTN_PLAY, &CAutoPressDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_CHK_ALWAYS_ON_TOP, &CAutoPressDlg::OnBnClickedChkAlwaysOnTop)
	ON_REGISTERED_MESSAGE(CAutoPressDlg::m_nCommunicationMessageId, &CAutoPressDlg::OnKeyPlayingMessage)
	ON_EN_CHANGE(IDC_EDT_REPEAT_TIMES, &CAutoPressDlg::OnEnChangeEdtRepeatTimes)
END_MESSAGE_MAP()


// CAutoPressDlg message handlers

BOOL CAutoPressDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	LOGFONT lf={0};
	GetFont()->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	lf.lfHeight = 24;
	m_fontStatus.CreateFontIndirect(&lf);
	m_sttPlayingStatus.SetFont(&m_fontStatus);

	CString strMessageName;
	strMessageName.Format(_T("%s%X%X"), AfxGetAppName(), AfxGetInstanceHandle(), GetCurrentProcessId());
	CAutoPressDlg::m_nCommunicationMessageId = RegisterWindowMessage(strMessageName);

	
	m_spinRepeatTimes.SetBuddy(&m_edtRepeatTimes);
	m_edtRepeatTimes.SetLimitText(2);
	m_spinRepeatTimes.SetRange(1, 100);
	m_spinRepeatTimes.SetPos(0);


	CheckDlgButton(IDC_RADIO_REPEAT_NONE, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_REPEAT_INFINITE, BST_UNCHECKED);
	CheckDlgButton(IDC_RADIO_REPEAT_NUMBER, BST_UNCHECKED);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAutoPressDlg::OnDestroy()
{
	if(m_pPlayThread != NULL)
	{
		if(m_pPlayThread->IsRunning())
			m_pPlayThread->Stop();
		delete m_pPlayThread;
	}

	CDialogEx::OnDestroy();
}

void CAutoPressDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

LRESULT CAutoPressDlg::OnKeyPlayingMessage(WPARAM wParam, LPARAM lParam)
{
	if(wParam == KBD_PLAY_EVENT_INIT)
	{
		if(!m_sttPlayingStatus.IsWindowVisible())
			m_sttPlayingStatus.ShowWindow(SW_SHOW);
	}
	else if(wParam == KBD_PLAY_EVENT_PLAYING)
	{
		KeyBdEvent* kbdEvent = (KeyBdEvent*)lParam;
		if(kbdEvent != NULL)
		{
			CString strKeyName;
			CString strStatus;

			if(!CKeyCodeMaper::Lookup(kbdEvent->VirtualKeyCode, strKeyName))
				strKeyName.Format(_T("0x%.2X"), kbdEvent->VirtualKeyCode);
			strStatus.Format(_T("%s %s"), strKeyName, kbdEvent->KeyUp == 0 ? _T("DOWN") : _T("UP"));
			if(m_sttPlayingStatus.GetSafeHwnd())
				m_sttPlayingStatus.SetWindowText(strStatus);
		}
	}
	else if(wParam == KBD_PLAY_EVENT_DEINIT)
	{
		if(m_sttPlayingStatus.IsWindowVisible())
			m_sttPlayingStatus.ShowWindow(SW_HIDE);

		m_btnRecord.EnableWindow(TRUE);
		m_btnPlay.SetWindowText(_T("&Play"));

		int nState = IsDlgButtonChecked(IDC_CHK_HIDE_WHEN_PLAY);
		if((nState & BST_CHECKED) != 0)
			ShowWindow(SW_RESTORE);
		if(m_pPlayThread != NULL)
		{
			if(m_pPlayThread->IsRunning())
				m_pPlayThread->Stop();
			delete m_pPlayThread;
			m_pPlayThread = NULL;
		}
	}

	return (LRESULT)0;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CAutoPressDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAutoPressDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAutoPressDlg::ClearKeyEvents()
{
	KeyBdEvent *event = NULL;
	for(INT_PTR i=0;i<m_arrKeyEvents.GetCount();i++)
	{
		event = (KeyBdEvent*)m_arrKeyEvents.GetAt(i);
		if(event != NULL)
			delete event;
	}
	m_arrKeyEvents.RemoveAll();
}

void CAutoPressDlg::OnBnClickedBtnClear()
{
	m_lstKeys.ResetContent();
	ClearKeyEvents();
	m_btnPlay.EnableWindow(m_lstKeys.GetCount() > 0);
}


void CAutoPressDlg::OnBnClickedBtnRecord()
{
	if(!m_bRecording) //Not recording
	{
		m_hKbdHook = ::SetWindowsHookEx(WH_KEYBOARD, &CAutoPressDlg::KeyboardProc, AfxGetInstanceHandle(), ::GetCurrentThreadId());
		if(m_hKbdHook != NULL)
		{
			m_lstKeys.ResetContent();
			m_dwLastTickCount = GetTickCount();
			m_bRecording = TRUE;
			m_btnRecord.SetWindowText(_T("&Stop"));
		}
	}
	else //Recording
	{
		if(m_hKbdHook != NULL)
		{
			::UnhookWindowsHookEx(m_hKbdHook);
			m_hKbdHook = NULL;
		}
		m_bRecording = FALSE;
		m_btnRecord.SetWindowText(_T("&Record"));
		m_btnPlay.EnableWindow(m_lstKeys.GetCount() > 0);
	}
}


void CAutoPressDlg::OnBnClickedBtnPlay()
{
	if(m_pPlayThread != NULL) //Playing -> Stop
	{
		if(m_pPlayThread->IsRunning())
		{
			m_pPlayThread->Stop();
		}
		WaitForSingleObject(m_pPlayThread->m_hThread, INFINITE);
		delete m_pPlayThread;
		m_pPlayThread = NULL;
		m_btnRecord.EnableWindow(TRUE);
		m_btnPlay.SetWindowText(_T("&Play"));
	}
	else //Not Playing -> Play
	{
		if(m_pPlayThread != NULL)
			delete m_pPlayThread;
		int nRepeatTimes = 0;
		int nRepeatOption = IsDlgButtonChecked(IDC_RADIO_REPEAT_NONE);
		if((nRepeatOption & BST_CHECKED) != 0)
			nRepeatTimes = 0;
		nRepeatOption = IsDlgButtonChecked(IDC_RADIO_REPEAT_NUMBER);
		if((nRepeatOption & BST_CHECKED) != 0)
			nRepeatTimes = GetDlgItemInt(IDC_EDT_REPEAT_TIMES);
		nRepeatOption = IsDlgButtonChecked(IDC_RADIO_REPEAT_INFINITE);
		if((nRepeatOption & BST_CHECKED) != 0)
			nRepeatTimes = -1;

		m_pPlayThread = new CKeysPlayThread(&m_arrKeyEvents, this, m_nCommunicationMessageId, nRepeatTimes);
		if(m_pPlayThread != NULL)
		{
			if(m_pPlayThread->CreateThread(CREATE_SUSPENDED, 0, NULL))
			{
				m_btnRecord.EnableWindow(FALSE);
				m_btnPlay.SetWindowText(_T("&Stop"));
				m_sttPlayingStatus.ShowWindow(SW_HIDE);

				int nState = IsDlgButtonChecked(IDC_CHK_HIDE_WHEN_PLAY);
				if((nState & BST_CHECKED) != 0)
					ShowWindow(SW_MINIMIZE);
				m_pPlayThread->ResumeThread();
			}
		}
	}
}


void CAutoPressDlg::OnBnClickedChkAlwaysOnTop()
{
	int nState = IsDlgButtonChecked(IDC_CHK_ALWAYS_ON_TOP);
	if((nState & BST_CHECKED) != 0)
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else
	{
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
}


void CAutoPressDlg::OnEnChangeEdtRepeatTimes()
{
	CheckDlgButton(IDC_RADIO_REPEAT_NUMBER, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_REPEAT_NONE, BST_UNCHECKED);
	CheckDlgButton(IDC_RADIO_REPEAT_INFINITE, BST_UNCHECKED);
}
