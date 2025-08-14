#pragma once
#include "afx.h"
class CKeyCodeMaper : public CObject
{
public:
	CKeyCodeMaper(void);
	virtual ~CKeyCodeMaper(void);
	static BOOL Lookup(DWORD dwKeyCode, CString& strKeyName);
private:
	static CMap<DWORD, DWORD, CString, CString> m_mapKeyToName;
};

