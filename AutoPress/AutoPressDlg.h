
// AutoPressDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

class CKeysPlayThread;
// CAutoPressDlg dialog
class CAutoPressDlg : public CDialogEx
{
// Construction
public:
	CAutoPressDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CAutoPressDlg();
// Dialog Data
	enum { IDD = IDD_AUTOPRESS_DIALOG };

	LRESULT OnKeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	static CAutoPressDlg* GetInstance() { return m_pInstance; }
	static LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
private:
	void ClearKeyEvents();
// Implementation
protected:
	HICON m_hIcon;
	BOOL m_bRecording;
	DWORD m_dwLastTickCount;
	HHOOK m_hKbdHook;
	CFont m_fontStatus;
	static CAutoPressDlg* m_pInstance;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	static UINT m_nCommunicationMessageId;
	CListBox m_lstKeys;
	CButton m_btnRecord;
	CButton m_btnClear;
	CButton m_btnPlay;
	afx_msg void OnBnClickedBtnClear();
	afx_msg void OnBnClickedBtnRecord();
	CPtrArray m_arrKeyEvents;
	afx_msg void OnBnClickedBtnPlay();
	CStatic m_sttPlayingStatus;
	CKeysPlayThread* m_pPlayThread;
	afx_msg void OnBnClickedChkAlwaysOnTop();
	afx_msg LRESULT OnKeyPlayingMessage(WPARAM wParam, LPARAM lParam);
	CEdit m_edtRepeatTimes;
	CSpinButtonCtrl m_spinRepeatTimes;
	afx_msg void OnEnChangeEdtRepeatTimes();
};
